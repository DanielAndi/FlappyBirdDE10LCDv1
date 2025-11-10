/**
 * @file gameplay.h
 * @brief Main gameplay loop and game logic
 * 
 * This module implements the main gameplay loop for the Flappy Bird game.
 * It handles game state, object updates, collision detection, rendering,
 * and user input processing.
 * 
 * @author Daniel Grijalva
 * @date 2025
 * @version 1.0
 */

#ifndef GAME_GAMEPLAY_H
#define GAME_GAMEPLAY_H

#include <signal.h>

#include "display/graphics.h"
#include "input/input_handler.h"
#include "hardware/hps_fpga_bridge.h"

/**
 * @brief Run the main gameplay loop
 * 
 * Initializes the game, runs the main game loop until the game ends or
 * the exit flag is set, and handles cleanup.
 * 
 * The game loop performs the following steps each frame:
 * 1. Poll input devices
 * 2. Update game objects (bird, pipes)
 * 3. Check for collisions
 * 4. Update score
 * 5. Render the frame
 * 6. Wait for next frame time
 * 
 * @param gfx Initialized graphics context
 * @param input Initialized input state
 * @param bridge Initialized FPGA bridge handle
 * @param should_exit Pointer to exit flag (checked each frame)
 * 
 * @note The function returns when the game ends or should_exit is set
 * @note This function performs null pointer checking for all parameters
 */
void gameplay_run(graphics_context_t *gfx,
                  input_state_t *input,
                  fpga_bridge_t *bridge,
                  volatile sig_atomic_t *should_exit);

#endif /* GAME_GAMEPLAY_H */

