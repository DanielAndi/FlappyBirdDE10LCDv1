/**
 * @file fpga_peripherals.c
 * @brief Implementation of FPGA peripheral access functions
 * 
 * This file implements the high-level functions for accessing FPGA peripherals
 * (LEDs, keys, switches, hex displays) through the HPS-FPGA bridge.
 * 
 * @author Daniel Grijalva
 * @date 2025
 * @version 1.0
 */

#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include "fpga_peripherals.h"
#include "address_map_arm.h"
#include "hps_fpga_bridge.h"
 
 int fpga_leds_init(fpga_bridge_t *bridge) {
     if (bridge == NULL || !bridge->initialized) {
         printf("FPGA LEDs: ERROR - bridge not initialized\n");
         return -1;
     }
     
     // Configure LEDR direction register: all outputs (0x3FF = all 10 bits set)
     fpga_peripheral_write(bridge, LEDR_BASE, PIO_DIR_OFFSET, 0x3FF);
     
     // Initialize all LEDs to off
     fpga_peripheral_write(bridge, LEDR_BASE, PIO_DATA_OFFSET, 0x0);
     
     printf("FPGA LEDs: Initialized (10 LEDs configured as outputs)\n");
     return 0;
 }
 
 void fpga_leds_set(fpga_bridge_t *bridge, uint32_t value) {
     if (bridge == NULL || !bridge->initialized) {
         return;
     }
     
     // Mask to 10 bits (only LEDs 0-9)
     value &= LED_ALL_MASK;
     fpga_peripheral_write(bridge, LEDR_BASE, PIO_DATA_OFFSET, value);
 }
 
 uint32_t fpga_leds_get(fpga_bridge_t *bridge) {
     if (bridge == NULL || !bridge->initialized) {
         return 0;
     }
     
     return fpga_peripheral_read(bridge, LEDR_BASE, PIO_DATA_OFFSET) & LED_ALL_MASK;
 }
 
 int fpga_keys_init(fpga_bridge_t *bridge) {
     if (bridge == NULL || !bridge->initialized) {
         printf("FPGA Keys: ERROR - bridge not initialized\n");
         return -1;
     }
     
     // Configure KEY direction register: all inputs (0x0)
     fpga_peripheral_write(bridge, KEY_BASE, PIO_DIR_OFFSET, 0x0);
     
     // Clear any latched edges
     fpga_peripheral_write(bridge, KEY_BASE, PIO_EDGE_OFFSET, KEYS_MASK);
     
     printf("FPGA Keys: Initialized (4 keys configured as inputs)\n");
     return 0;
 }
 
 uint32_t fpga_keys_read(fpga_bridge_t *bridge) {
     if (bridge == NULL || !bridge->initialized) {
         return KEYS_MASK;  // Return "all not pressed" if bridge not initialized
     }
     
     // Read key state and mask to 4 bits
     // Keys are active-low: 0 = pressed, 1 = not pressed
     return fpga_peripheral_read(bridge, KEY_BASE, PIO_DATA_OFFSET) & KEYS_MASK;
 }
 
 bool fpga_keys_any_pressed(fpga_bridge_t *bridge) {
     uint32_t keys = fpga_keys_read(bridge);
     // Keys are active-low, so if any bit is 0, a key is pressed
     // When no keys pressed, all bits are 1 (0xF)
     return (keys != KEYS_MASK);
 }
 
 void fpga_keys_clear_edges(fpga_bridge_t *bridge) {
     if (bridge == NULL || !bridge->initialized) {
         return;
     }
     
     // Clear edge capture register by writing the mask
     fpga_peripheral_write(bridge, KEY_BASE, PIO_EDGE_OFFSET, KEYS_MASK);
 }
 
int fpga_switches_init(fpga_bridge_t *bridge) {
    if (bridge == NULL || !bridge->initialized) {
        printf("FPGA Switches: ERROR - bridge not initialized\n");
        return -1;
    }

    fpga_peripheral_write(bridge, SW_BASE, PIO_DIR_OFFSET, 0x0);

    printf("FPGA Switches: Initialized (10 switches configured as inputs)\n");
    return 0;
}

uint32_t fpga_switches_read(fpga_bridge_t *bridge) {
    if (bridge == NULL || !bridge->initialized) {
        return SWITCHES_MASK;
    }

    return fpga_peripheral_read(bridge, SW_BASE, PIO_DATA_OFFSET) & SWITCHES_MASK;
}

 void fpga_leds_blink(fpga_bridge_t *bridge, int count, unsigned int delay_us) {
     if (bridge == NULL || !bridge->initialized) {
         return;
     }
     
     for (int i = 0; i < count; i++) {
         fpga_leds_set(bridge, LED0_BIT);   // LED0 on
         usleep(delay_us);
         fpga_leds_set(bridge, 0x0);        // All LEDs off
         usleep(delay_us);
     }
 }

// 7-segment display encoding lookup table
// Each value represents which segments are lit for digits 0-9 and A-F
// Bit encoding: 0=a, 1=b, 2=c, 3=d, 4=e, 5=f, 6=g, 7=decimal point
static const uint8_t HEX_SEGMENTS[16] = {
    0x3F,  // 0: segments a,b,c,d,e,f
    0x06,  // 1: segments b,c
    0x5B,  // 2: segments a,b,d,e,g
    0x4F,  // 3: segments a,b,c,d,g
    0x66,  // 4: segments b,c,f,g
    0x6D,  // 5: segments a,c,d,f,g
    0x7D,  // 6: segments a,c,d,e,f,g
    0x07,  // 7: segments a,b,c
    0x7F,  // 8: segments a,b,c,d,e,f,g
    0x67,  // 9: segments a,b,c,d,f,g
    0x77,  // A: segments a,b,c,e,f,g
    0x7C,  // B: segments c,d,e,f,g
    0x39,  // C: segments a,d,e,f
    0x5E,  // D: segments b,c,d,e,g
    0x79,  // E: segments a,d,e,f,g
    0x71   // F: segments a,e,f,g
};

int fpga_hex_init(fpga_bridge_t *bridge) {
    if (bridge == NULL || !bridge->initialized) {
        printf("FPGA Hex Display: ERROR - bridge not initialized\n");
        return -1;
    }
    
    // Clear all hex displays
    fpga_peripheral_write(bridge, HEX3_HEX0_BASE, PIO_DATA_OFFSET, 0x0);
    fpga_peripheral_write(bridge, HEX5_HEX4_BASE, PIO_DATA_OFFSET, 0x0);
    
    printf("FPGA Hex Display: Initialized (6 displays cleared)\n");
    return 0;
}

void fpga_hex_display(fpga_bridge_t *bridge, uint32_t value) {
    if (bridge == NULL || !bridge->initialized) {
        return;
    }
    
    // Extract individual decimal digits (0-9 for HEX0-HEX5)
    // Convert value to decimal digits by repeatedly dividing by 10
    uint8_t digits[6] = {0, 0, 0, 0, 0, 0};
    uint32_t temp = value;
    
    // Extract decimal digits (least significant first)
    for (int i = 0; i < 6 && temp > 0; i++) {
        digits[i] = (uint8_t)(temp % 10);
        temp /= 10;
    }
    
    // Convert decimal digits to 7-segment encoding
    uint8_t digit0 = HEX_SEGMENTS[digits[0]];  // HEX0: ones place
    uint8_t digit1 = HEX_SEGMENTS[digits[1]];  // HEX1: tens place
    uint8_t digit2 = HEX_SEGMENTS[digits[2]];  // HEX2: hundreds place
    uint8_t digit3 = HEX_SEGMENTS[digits[3]];  // HEX3: thousands place
    uint8_t digit4 = HEX_SEGMENTS[digits[4]];  // HEX4: ten thousands place
    uint8_t digit5 = HEX_SEGMENTS[digits[5]];  // HEX5: hundred thousands place
    
    // Pack digits into 32-bit values
    // HEX3_HEX0: [31:24]=HEX3, [23:16]=HEX2, [15:8]=HEX1, [7:0]=HEX0
    uint32_t hex3_hex0 = ((uint32_t)digit3 << 24) |
                         ((uint32_t)digit2 << 16) |
                         ((uint32_t)digit1 << 8) |
                         ((uint32_t)digit0);
    
    // HEX5_HEX4: [15:8]=HEX5, [7:0]=HEX4
    uint32_t hex5_hex4 = ((uint32_t)digit5 << 8) |
                         ((uint32_t)digit4);
    
    // Write to hex display registers
    fpga_peripheral_write(bridge, HEX3_HEX0_BASE, PIO_DATA_OFFSET, hex3_hex0);
    fpga_peripheral_write(bridge, HEX5_HEX4_BASE, PIO_DATA_OFFSET, hex5_hex4);
}

void fpga_hex_clear(fpga_bridge_t *bridge) {
    if (bridge == NULL || !bridge->initialized) {
        return;
    }
    
    fpga_peripheral_write(bridge, HEX3_HEX0_BASE, PIO_DATA_OFFSET, 0x0);
    fpga_peripheral_write(bridge, HEX5_HEX4_BASE, PIO_DATA_OFFSET, 0x0);
}
 