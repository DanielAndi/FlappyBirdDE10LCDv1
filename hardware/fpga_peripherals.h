/**
 * @file fpga_peripherals.h
 * @brief FPGA Peripheral Abstraction Layer
 * 
 * This module provides high-level functions to interact with FPGA peripherals
 * (LEDs, Keys, Switches, etc.) through the HPS-FPGA bridge.
 * 
 * These functions abstract the low-level register access and provide
 * a clean API for application code.
 */

 #ifndef FPGA_PERIPHERALS_H
 #define FPGA_PERIPHERALS_H
 
 #include <stdint.h>
 #include <stdbool.h>
 #include "hps_fpga_bridge.h"
 
 // PIO register offsets (relative to each peripheral base)
 #define PIO_DATA_OFFSET   0x0   ///< Data register offset
 #define PIO_DIR_OFFSET    0x4   ///< Direction register offset
 #define PIO_EDGE_OFFSET   0xC   ///< Edge capture register offset
 
 // LED definitions
 #define LED0_BIT          (1u << 0)   ///< LED0 bit mask
 #define LED_ALL_MASK      0x3FF       ///< All 10 LEDs mask
 
// Key definitions
 #define KEYS_MASK         0xFu        ///< All 4 keys mask
 #define KEY0_BIT          (1u << 0)   ///< KEY0 bit mask
 #define KEY1_BIT          (1u << 1)   ///< KEY1 bit mask
 #define KEY2_BIT          (1u << 2)   ///< KEY2 bit mask
 #define KEY3_BIT          (1u << 3)   ///< KEY3 bit mask

// Switch definitions
#define SWITCHES_MASK     0x3FFu      ///< All 10 switches mask
 
 /**
  * @brief Initialize FPGA LEDs (LEDR)
  * 
  * Configures the LEDR peripheral as outputs and turns all LEDs off.
  * 
  * @param bridge Initialized FPGA bridge handle
  * @return 0 on success, -1 on failure
  */
 int fpga_leds_init(fpga_bridge_t *bridge);
 
 /**
  * @brief Set LED state
  * 
  * Sets the state of all LEDs. Each bit corresponds to one LED.
  * 
  * @param bridge Initialized FPGA bridge handle
  * @param value Bitmask where bit N controls LED N (0=off, 1=on)
  */
 void fpga_leds_set(fpga_bridge_t *bridge, uint32_t value);
 
 /**
  * @brief Get current LED state
  * 
  * @param bridge Initialized FPGA bridge handle
  * @return Current LED state as a bitmask
  */
 uint32_t fpga_leds_get(fpga_bridge_t *bridge);
 
 /**
  * @brief Initialize FPGA Keys (KEY)
  * 
  * Configures the KEY peripheral as inputs and clears any latched edges.
  * 
  * @param bridge Initialized FPGA bridge handle
  * @return 0 on success, -1 on failure
  */
 int fpga_keys_init(fpga_bridge_t *bridge);
 
 /**
  * @brief Read key state
  * 
  * Reads the current state of all keys.
  * Keys are active-low: 0 = pressed, 1 = not pressed
  * 
  * @param bridge Initialized FPGA bridge handle
  * @return Key state as a bitmask (bits 0-3 correspond to KEY0-KEY3)
  *         A pressed key has its bit cleared (0), not pressed has bit set (1)
  */
 uint32_t fpga_keys_read(fpga_bridge_t *bridge);
 
 /**
  * @brief Check if any key is pressed
  * 
  * @param bridge Initialized FPGA bridge handle
  * @return true if any key is pressed, false otherwise
  */
 bool fpga_keys_any_pressed(fpga_bridge_t *bridge);
 
 /**
  * @brief Clear edge capture register for keys
  * 
  * Clears any latched edge captures in the KEY peripheral.
  * This should be called after handling a key press to clear the edge register.
  * 
  * @param bridge Initialized FPGA bridge handle
  */
 void fpga_keys_clear_edges(fpga_bridge_t *bridge);

/**
 * @brief Initialize FPGA Switches (SW)
 *
 * Configures the switch peripheral as inputs.
 *
 * @param bridge Initialized FPGA bridge handle
 * @return 0 on success, -1 on failure
 */
int fpga_switches_init(fpga_bridge_t *bridge);

/**
 * @brief Read switch state
 *
 * Reads the current state of all switches.
 *
 * @param bridge Initialized FPGA bridge handle
 * @return Switch state as a bitmask (bits 0-9 correspond to switches 0-9)
 */
uint32_t fpga_switches_read(fpga_bridge_t *bridge);
 
 /**
  * @brief Blink LEDs a specified number of times
  * 
  * Blinks LED0 on and off for the specified number of times.
  * 
  * @param bridge Initialized FPGA bridge handle
  * @param count Number of times to blink
  * @param delay_us Delay in microseconds between on/off states
  */
 void fpga_leds_blink(fpga_bridge_t *bridge, int count, unsigned int delay_us);

/**
 * @brief Initialize hex displays
 * 
 * Initializes the hex displays and clears them (turns all segments off).
 * 
 * @param bridge Initialized FPGA bridge handle
 * @return 0 on success, -1 on failure
 */
int fpga_hex_init(fpga_bridge_t *bridge);

/**
 * @brief Display a value on hex displays
 * 
 * Displays a numeric value on the hex displays. The value is displayed
 * in hexadecimal format, with the least significant digit on HEX0.
 * 
 * @param bridge Initialized FPGA bridge handle
 * @param value Value to display (will be shown in hex format)
 */
void fpga_hex_display(fpga_bridge_t *bridge, uint32_t value);

/**
 * @brief Clear all hex displays
 * 
 * Turns off all segments on all hex displays.
 * 
 * @param bridge Initialized FPGA bridge handle
 */
void fpga_hex_clear(fpga_bridge_t *bridge);
 
 #endif /* FPGA_PERIPHERALS_H */
 
 