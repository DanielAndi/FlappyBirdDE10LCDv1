/**
 * @file lcd_driver.c
 * @brief LCD driver leveraging HPS GPIO and SPI peripherals for the DE10-Standard.
 *
 * This implementation targets the onboard 128x64 monochrome LCD driven by an
 * NT7534-compatible controller. Communication uses the HPS internal GPIO1 block
 * for control signals (D/C, RESET, BACKLIGHT) and the HPS SPIM0 controller for
 * serial data. A simple 1bpp frame buffer is maintained in software; drawing
 * operations update the buffer and `lcd_driver_flush` pushes it to the panel.
 */

#include "lcd_driver.h"

#include "address_map_arm.h"

#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define LCD_PAGE_COUNT (LCD_HEIGHT / 8U)

/* GPIO register offsets relative to HPS_BRIDGE */
#define HPS_GPIO1_DR_OFFSET   0x0000
#define HPS_GPIO1_DDR_OFFSET  0x0004

/* GPIO bit assignments */
#define GPIO_DC_BIT        (1U << 12) /* GPIO1[41] */
#define GPIO_RESETN_BIT    (1U << 15) /* GPIO1[44] */
#define GPIO_BACKLIGHT_BIT (1U << 8)  /* GPIO1[37] */

/* SPIM0 register offsets */
#define SPIM_CTLR0   0x0000
#define SPIM_SPIENR  0x0008
#define SPIM_SER     0x0010
#define SPIM_BAUDR   0x0014
#define SPIM_SR      0x0028
#define SPIM_DR      0x0060

/* SPIM0 bit masks */
#define SPIM_CTLR0_TMOD_MASK  (0x3U << 8)
#define SPIM_CTLR0_TMOD_TX    (0x1U << 8)
#define SPIM_SPIENR_ENABLE    (1U << 0)
#define SPIM_SER_SS0          (1U << 0)
#define SPIM_SR_TFE           (1U << 2)
#define SPIM_SR_BUSY          (1U << 0)

/* LCD command set */
#define LCD_CMD_DISPLAY_ON      0xAF
#define LCD_CMD_DISPLAY_OFF     0xAE
#define LCD_CMD_SET_START_LINE  0x40
#define LCD_CMD_SET_PAGE        0xB0
#define LCD_CMD_SET_COL_LOW     0x00
#define LCD_CMD_SET_COL_HIGH    0x10
#define LCD_CMD_SET_ADC_NORMAL  0xA0
#define LCD_CMD_SET_BIAS        0xA2
#define LCD_CMD_SET_COM_NORMAL  0xC0
#define LCD_CMD_POWER_CONTROL   0x28
#define LCD_CMD_BOOSTER_ON      0x2F
#define LCD_CMD_RESISTOR_RATIO  0x20

static int lcd_initialized = 0;
static int mem_fd = -1;
static void *hps_virtual_base = NULL;
static void *spim0_base = NULL;
static uint8_t lcd_buffer[LCD_PAGE_COUNT][LCD_WIDTH];

static inline volatile uint32_t *gpio1_reg(uint32_t offset) {
    return (volatile uint32_t *)((uint8_t *)hps_virtual_base + HPS_GPIO1_BASE + offset);
}

static inline volatile uint32_t *spim0_reg(uint32_t offset) {
    return (volatile uint32_t *)((uint8_t *)spim0_base + offset);
}

static inline void setbits32(volatile uint32_t *addr, uint32_t mask) {
    *addr |= mask;
}

static inline void clrbits32(volatile uint32_t *addr, uint32_t mask) {
    *addr &= ~mask;
}

static bool spim_tx_empty(void) {
    volatile uint32_t *sr = spim0_reg(SPIM_SR);
    return (*sr & SPIM_SR_TFE) != 0U;
}

static bool spim_busy(void) {
    volatile uint32_t *sr = spim0_reg(SPIM_SR);
    return (*sr & SPIM_SR_BUSY) != 0U;
}

static void spim_write(uint8_t value) {
    volatile uint32_t *dr = spim0_reg(SPIM_DR);
    while (!spim_tx_empty()) {
        usleep(1);
    }
    *dr = (uint32_t)value;
    while (!spim_tx_empty()) {
        usleep(1);
    }
    while (spim_busy()) {
        usleep(1);
    }
}

static void lcd_set_dc(bool data) {
    volatile uint32_t *dr = gpio1_reg(HPS_GPIO1_DR_OFFSET);
    if (data) {
        setbits32(dr, GPIO_DC_BIT);
    } else {
        clrbits32(dr, GPIO_DC_BIT);
    }
}

static void lcd_reset(void) {
    volatile uint32_t *dr = gpio1_reg(HPS_GPIO1_DR_OFFSET);
    volatile uint32_t *ddr = gpio1_reg(HPS_GPIO1_DDR_OFFSET);
    setbits32(ddr, GPIO_RESETN_BIT);
    clrbits32(dr, GPIO_RESETN_BIT);
    usleep(5000);
    setbits32(dr, GPIO_RESETN_BIT);
    usleep(5000);
}

static void lcd_backlight(bool on) {
    volatile uint32_t *dr = gpio1_reg(HPS_GPIO1_DR_OFFSET);
    volatile uint32_t *ddr = gpio1_reg(HPS_GPIO1_DDR_OFFSET);
    setbits32(ddr, GPIO_BACKLIGHT_BIT);
    if (on) {
        setbits32(dr, GPIO_BACKLIGHT_BIT);
    } else {
        clrbits32(dr, GPIO_BACKLIGHT_BIT);
    }
}

static void lcd_write_command(uint8_t cmd) {
    lcd_set_dc(false);
    spim_write(cmd);
}

static void lcd_write_data(uint8_t data) {
    lcd_set_dc(true);
    spim_write(data);
}

static void lcd_set_page(uint8_t page) {
    lcd_write_command((uint8_t)(LCD_CMD_SET_PAGE | (page & 0x0F)));
}

static void lcd_set_column(uint8_t column) {
    lcd_write_command((uint8_t)(LCD_CMD_SET_COL_LOW | (column & 0x0F)));
    lcd_write_command((uint8_t)(LCD_CMD_SET_COL_HIGH | ((column >> 4) & 0x0F)));
}

static void lcd_flush_page(uint8_t page) {
    lcd_set_page(page);
    lcd_set_column(0);
    for (uint32_t col = 0; col < LCD_WIDTH; ++col) {
        lcd_write_data(lcd_buffer[page][col]);
    }
}

static void lcd_hw_cleanup(void) {
    if (spim0_base != NULL) {
        munmap(spim0_base, SPIM0_SPAN);
        spim0_base = NULL;
    }
    if (hps_virtual_base != NULL) {
        munmap(hps_virtual_base, HPS_BRIDGE_SPAN);
        hps_virtual_base = NULL;
    }
    if (mem_fd != -1) {
        close(mem_fd);
        mem_fd = -1;
    }
    lcd_initialized = 0;
}

static int lcd_hw_init(void) {
    if (lcd_initialized) {
        return 0;
    }

    mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (mem_fd == -1) {
        printf("LCD: ERROR - unable to open /dev/mem\n");
        return -1;
    }

    hps_virtual_base = mmap(NULL,
                            HPS_BRIDGE_SPAN,
                            PROT_READ | PROT_WRITE,
                            MAP_SHARED,
                            mem_fd,
                            HPS_BRIDGE_BASE);
    if (hps_virtual_base == MAP_FAILED) {
        printf("LCD: ERROR - mmap failed for HPS bridge\n");
        lcd_hw_cleanup();
        return -1;
    }

    spim0_base = mmap(NULL,
                      SPIM0_SPAN,
                      PROT_READ | PROT_WRITE,
                      MAP_SHARED,
                      mem_fd,
                      SPIM0_BASE);
    if (spim0_base == MAP_FAILED) {
        printf("LCD: ERROR - mmap failed for SPIM0\n");
        lcd_hw_cleanup();
        return -1;
    }

    /* Configure DC pin as output and default to command mode */
    volatile uint32_t *gpio1_ddr = gpio1_reg(HPS_GPIO1_DDR_OFFSET);
    volatile uint32_t *gpio1_dr = gpio1_reg(HPS_GPIO1_DR_OFFSET);
    setbits32(gpio1_ddr, GPIO_DC_BIT);
    clrbits32(gpio1_dr, GPIO_DC_BIT);

    /* Initialize SPIM0: disable, configure TX-only, set divisor, enable SS0 */
    volatile uint32_t *ctlr0 = spim0_reg(SPIM_CTLR0);
    volatile uint32_t *spienr = spim0_reg(SPIM_SPIENR);
    volatile uint32_t *baudr = spim0_reg(SPIM_BAUDR);
    volatile uint32_t *ser = spim0_reg(SPIM_SER);

    clrbits32(spienr, SPIM_SPIENR_ENABLE);
    clrbits32(ctlr0, SPIM_CTLR0_TMOD_MASK);
    setbits32(ctlr0, SPIM_CTLR0_TMOD_TX);
    clrbits32(baudr, 0xFFFFU);
    setbits32(baudr, 64U); /* 200 MHz / 64 ≈ 3.125 MHz */
    clrbits32(ser, 0xFU);
    setbits32(ser, SPIM_SER_SS0);
    setbits32(spienr, SPIM_SPIENR_ENABLE);

    lcd_reset();
    lcd_backlight(false);

    /* Basic init sequence */
    lcd_write_command(LCD_CMD_SET_ADC_NORMAL);
    lcd_write_command(LCD_CMD_SET_BIAS);
    lcd_write_command(LCD_CMD_SET_COM_NORMAL);
    lcd_write_command((uint8_t)(LCD_CMD_POWER_CONTROL | 0x07));
    lcd_write_command(LCD_CMD_BOOSTER_ON);
    lcd_write_command((uint8_t)(LCD_CMD_RESISTOR_RATIO | 0x04));
    lcd_write_command(LCD_CMD_SET_START_LINE);
    lcd_write_command(LCD_CMD_DISPLAY_ON);
    lcd_backlight(true);

    memset(lcd_buffer, 0, sizeof(lcd_buffer));
    for (uint8_t page = 0; page < LCD_PAGE_COUNT; ++page) {
        lcd_flush_page(page);
    }

    lcd_initialized = 1;
    return 0;
}

int lcd_driver_init(void) {
    if (lcd_hw_init() != 0) {
        return -1;
    }
    return 0;
}

void lcd_driver_shutdown(void) {
    if (!lcd_initialized) {
        return;
    }

    lcd_write_command(LCD_CMD_DISPLAY_OFF);
    lcd_backlight(false);
    lcd_hw_cleanup();
}

void lcd_driver_clear(void) {
    if (!lcd_initialized) {
        return;
    }

    memset(lcd_buffer, 0, sizeof(lcd_buffer));
    for (uint8_t page = 0; page < LCD_PAGE_COUNT; ++page) {
        lcd_flush_page(page);
    }
}

void lcd_driver_draw_pixel(uint32_t x, uint32_t y, uint8_t value) {
    if (!lcd_initialized) {
        return;
    }
    if (x >= LCD_WIDTH || y >= LCD_HEIGHT) {
        return;
    }

    uint32_t page = y / 8U;
    uint8_t bit = (uint8_t)(1U << (y % 8U));

    if (value) {
        lcd_buffer[page][x] |= bit;
    } else {
        lcd_buffer[page][x] &= (uint8_t)(~bit);
    }
}

void lcd_driver_flush(void) {
    if (!lcd_initialized) {
        return;
    }

    for (uint8_t page = 0; page < LCD_PAGE_COUNT; ++page) {
        lcd_flush_page(page);
    }
}

