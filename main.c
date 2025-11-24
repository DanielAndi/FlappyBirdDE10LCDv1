/**
 * @file main.c
 * @brief Main entry point for Flappy Bird game on DE10-Standard board
 * 
 * This file implements the main game loop and initialization sequence for the
 * Flappy Bird game running on the DE10-Standard FPGA development board.
 * The game uses the HPS (ARM Cortex-A9) processor to run the game logic and
 * communicates with FPGA peripherals through the lightweight bridge.
 * 
 * @author Daniel Grijalva
 * @date 2025
 * @version 1.0
 * 
 * @target_platform DE10-Standard FPGA Board (Cyclone V SoC)
 * @target_os Embedded Linux
 * @compiler ARM GCC
 * 
 * Architecture:
 * ------------
 * 1. Initialize FPGA bridge and hardware peripherals
 * 2. Initialize graphics subsystem and display startup screen
 * 3. Initialize input handler for button/switch reading
 * 4. Wait for user input to start game
 * 5. Run main gameplay loop
 * 6. Cleanup and exit gracefully
 */

#define _POSIX_C_SOURCE 200809L

#include <signal.h>
#include <stdio.h>
#include <stdint.h>

#include "hardware/hps_fpga_bridge.h"
#include "hardware/fpga_peripherals.h"
#include "display/graphics.h"
#include "input/input_handler.h"
#include "game/game_state.h"
#include "game/gameplay.h"

/** Global flag indicating the program should exit */
static volatile sig_atomic_t g_should_exit = 0;

/** Global flag indicating a signal was received */
static volatile sig_atomic_t g_signal_received = 0;

/**
 * @brief Signal handler for graceful program shutdown
 * 
 * Handles SIGINT (Ctrl+C) and SIGTERM signals to allow the program to
 * exit cleanly. Sets the global exit flags to stop the main loop.
 * 
 * @param signum Signal number (unused, but required by signal handler signature)
 */
static void handle_signal(int signum) {
    (void)signum;
    g_signal_received = 1;
    g_should_exit = 1;
}

/**
 * @brief Main entry point for the Flappy Bird game
 * 
 * Initializes all subsystems, runs the game loop, and performs cleanup.
 * The function follows this sequence:
 * 1. Install signal handlers for graceful shutdown
 * 2. Initialize FPGA bridge and peripherals
 * 3. Initialize graphics and display startup screen
 * 4. Initialize input handling
 * 5. Wait for user input to start
 * 6. Run main gameplay loop
 * 7. Cleanup and exit
 * 
 * @return 0 on successful execution, 1 on initialization failure
 * 
 * @note Requires root privileges to access /dev/mem for FPGA bridge
 * @note The program can be interrupted with SIGINT (Ctrl+C) or SIGTERM
 */
int main(void) {
    // Install signal handlers for graceful shutdown
    struct sigaction sa;
    sa.sa_handler = handle_signal;
    sa.sa_flags = 0;
    if (sigemptyset(&sa.sa_mask) != 0 ||
        sigaction(SIGINT, &sa, NULL) != 0 ||
        sigaction(SIGTERM, &sa, NULL) != 0) {
        fprintf(stderr, "Failed to install signal handlers\n");
        return 1;
    }

    // Initialize FPGA bridge (required for all hardware access)
    fpga_bridge_t bridge;
    if (fpga_bridge_init(&bridge) != 0) {
        fprintf(stderr, "Failed to initialize FPGA bridge\n");
        return 1;
    }

    // Initialize LEDs
    if (fpga_leds_init(&bridge) != 0) {
        fprintf(stderr, "Failed to initialize LEDs\n");
        fpga_bridge_cleanup(&bridge);
        return 1;
    }

    // Initialize game state early to load high score
    game_state_t game_state;
    game_state_init(&game_state);
    game_state_set(&game_state);

    // Initialize graphics subsystem and display startup screen
    graphics_context_t graphics;
    graphics_init(&graphics);
    
    // Draw startup screen with high score if available
    graphics_clear(&graphics);
    graphics_draw_text(&graphics, 16U, 16U, "HELLO!");
    graphics_draw_text(&graphics, 8U, 32U, "PRESS BUTTON");
    int high_score = game_state_get_high_score();
    if (high_score > 0) {
        graphics_draw_text(&graphics, 8U, 48U, "HI:");
        graphics_draw_number(&graphics, 40U, 48U, (uint32_t)high_score);
    }
    graphics_present(&graphics);

    // Initialize input handler for button/switch reading
    input_state_t input;
    if (input_handler_init(&input, &bridge) != 0) {
        fprintf(stderr, "Failed to initialize input handler\n");
        fpga_bridge_cleanup(&bridge);
        return 1;
    }

    // Wait for user to press a button to start the game
    input_handler_wait_for_button_press(&input, &bridge, &g_should_exit);
    if (!g_should_exit) {
        input_handler_wait_for_button_release(&input, &bridge, &g_should_exit);
    }
    
    // Run the main gameplay loop
    if (!g_should_exit) {
        gameplay_run(&graphics, &input, &bridge, &g_should_exit);
    }

    // Cleanup: Turn off LEDs and hex displays
    fpga_leds_set(&bridge, 0U);
    fpga_hex_clear(&bridge);
    
    // Clear screen if we didn't receive a signal (graceful exit)
    if (!g_signal_received) {
        graphics_clear(&graphics);
        graphics_present(&graphics);
    }
    
    // Shutdown graphics subsystem
    graphics_shutdown(&graphics);
    
    // Cleanup FPGA bridge
    fpga_bridge_cleanup(&bridge);

    return 0;
}
