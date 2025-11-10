/**
 * @file lcd_driver.h
 * @brief Low-level LCD driver for DE10-Standard board
 * 
 * This module provides low-level access to the on-board 128×64 monochrome LCD
 * display. It uses the HPS SPI0 controller for data communication and HPS GPIO1
 * for control signals (D/C, RESET, BACKLIGHT).
 * 
 * The driver maintains an internal 1-bit per pixel frame buffer that is flushed
 * to the display when requested.
 * 
 * @author Daniel Grijalva
 * @date 2025
 * @version 1.0
 * 
 * @note The LCD controller is NT7534-compatible
 * @note Display resolution: 128×64 pixels (128 columns × 64 rows)
 */

#ifndef LCD_DRIVER_H
#define LCD_DRIVER_H

#include <stdint.h>

/** LCD display width in pixels */
#define LCD_WIDTH 128U

/** LCD display height in pixels */
#define LCD_HEIGHT 64U

/**
 * @brief Initialize the LCD driver
 * 
 * Initializes the HPS SPI0 and GPIO1 peripherals, configures the LCD controller,
 * and clears the display. This must be called before any other LCD driver functions.
 * 
 * @return 0 on success, -1 on failure
 * 
 * @note Requires root privileges to access /dev/mem
 */
int lcd_driver_init(void);

/**
 * @brief Shutdown the LCD driver
 * 
 * Cleans up the LCD driver and releases resources. This should be called
 * before program exit.
 * 
 * @note This function performs cleanup but does not clear the display
 */
void lcd_driver_shutdown(void);

/**
 * @brief Clear the entire display frame buffer
 * 
 * Sets all pixels in the internal frame buffer to off (0).
 * The display is not updated until lcd_driver_flush() is called.
 */
void lcd_driver_clear(void);

/**
 * @brief Draw a single pixel in the frame buffer
 * 
 * Sets a pixel in the internal frame buffer. The pixel value is 0 (off) or 1 (on).
 * 
 * @param x X coordinate (0 to LCD_WIDTH-1)
 * @param y Y coordinate (0 to LCD_HEIGHT-1)
 * @param value Pixel value (0 = off, 1 = on)
 * 
 * @note Coordinates are clipped to display bounds
 * @note The display is not updated until lcd_driver_flush() is called
 */
void lcd_driver_draw_pixel(uint32_t x, uint32_t y, uint8_t value);

/**
 * @brief Flush the frame buffer to the LCD display
 * 
 * Transmits the entire frame buffer to the LCD display via SPI.
 * This function should be called after drawing operations to update
 * the visible display.
 * 
 * @note This function is blocking and may take several milliseconds
 */
void lcd_driver_flush(void);

#endif /* LCD_DRIVER_H */

