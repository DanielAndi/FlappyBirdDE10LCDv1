/**
 * @file input_handler.h
 * @brief Button and switch input abstraction layer for gameplay
 * 
 * This module provides a high-level interface for reading and processing
 * user input from the DE10-Standard board's pushbuttons (KEY) and switches (SW).
 * It abstracts the low-level FPGA peripheral access and provides convenient
 * functions for common input operations like polling and waiting for button presses.
 * 
 * @author Daniel Grijalva
 * @date 2025
 * @version 1.0
 * 
 * Hardware Interface:
 * -------------------
 * - KEY[3:0]: Pushbuttons (active-low, 0 = pressed, 1 = not pressed)
 *   - KEY0: Primary jump/flap button
 *   - KEY1: Reset button (future use)
 *   - KEY2-KEY3: Reserved for future use
 * - SW[9:0]: Slide switches for game configuration
 * 
 * Usage Example:
 * --------------
 * @code
 * input_state_t input;
 * fpga_bridge_t bridge;
 * // ... initialize bridge ...
 * 
 * if (input_handler_init(&input, &bridge) == 0) {
 *     input_handler_poll(&input, &bridge);
 *     if (input_handler_is_jump_pressed(&input)) {
 *         // Handle jump action
 *     }
 * }
 * @endcode
 */

#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <signal.h>
#include <stdbool.h>
#include <stdint.h>

#include "hardware/hps_fpga_bridge.h"

/**
 * @brief Input state structure containing current button and switch states
 * 
 * This structure stores the current state of all input devices.
 * The keys and switches values are bitmasks where each bit represents
 * the state of one input device.
 */
typedef struct {
    uint32_t keys;      /**< Current state of all keys (KEY[3:0]) */
    uint32_t switches;  /**< Current state of all switches (SW[9:0]) */
} input_state_t;

/**
 * @brief Initialize the input handler subsystem
 * 
 * Initializes the FPGA keys and switches peripherals and performs
 * an initial poll to populate the input state.
 * 
 * @param state Pointer to input state structure to initialize
 * @param bridge Initialized FPGA bridge handle
 * @return 0 on success, -1 on failure
 * 
 * @note This function must be called before using any other input handler functions
 * @note The bridge must be initialized before calling this function
 */
int input_handler_init(input_state_t *state, fpga_bridge_t *bridge);

/**
 * @brief Poll input devices and update the input state
 * 
 * Reads the current state of all keys and switches from the FPGA
 * peripherals and updates the input state structure.
 * 
 * @param state Pointer to input state structure to update
 * @param bridge Initialized FPGA bridge handle
 * 
 * @note This function should be called regularly in the game loop
 *       to keep input state up-to-date
 */
void input_handler_poll(input_state_t *state, fpga_bridge_t *bridge);

/**
 * @brief Check if the jump button (KEY0) is currently pressed
 * 
 * @param state Pointer to input state structure
 * @return true if KEY0 is pressed, false otherwise
 * 
 * @note Keys are active-low, so a pressed key has its bit cleared (0)
 */
bool input_handler_is_jump_pressed(const input_state_t *state);

/**
 * @brief Wait for a button press
 * 
 * Blocks until a button is pressed or the exit flag is set.
 * This function first waits for any currently pressed button to be released,
 * then waits for a new button press.
 * 
 * @param state Pointer to input state structure
 * @param bridge Initialized FPGA bridge handle
 * @param should_exit Pointer to exit flag (can be NULL if not needed)
 * 
 * @note This function will poll buttons periodically while waiting
 * @note Returns immediately if should_exit is set (if not NULL)
 */
void input_handler_wait_for_button_press(input_state_t *state, fpga_bridge_t *bridge,
                                         volatile sig_atomic_t *should_exit);

/**
 * @brief Wait for a button to be released
 * 
 * Blocks until all buttons are released or the exit flag is set.
 * 
 * @param state Pointer to input state structure
 * @param bridge Initialized FPGA bridge handle
 * @param should_exit Pointer to exit flag (can be NULL if not needed)
 * 
 * @note This function will poll buttons periodically while waiting
 * @note Returns immediately if should_exit is set (if not NULL)
 */
void input_handler_wait_for_button_release(input_state_t *state, fpga_bridge_t *bridge,
                                           volatile sig_atomic_t *should_exit);

#endif /* INPUT_HANDLER_H */