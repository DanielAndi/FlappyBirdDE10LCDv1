/**
 * @file game_state.c
 * @brief Implementation of game state management functions for Flappy Bird on DE10-Standard
 * 
 * This file implements all the game state management functions declared in game_state.h.
 * It provides the core functionality for initializing, updating, and accessing game
 * state variables in the Flappy Bird implementation running on the DE10-Standard
 * FPGA board with embedded Linux.
 * 
 * @details The implementation focuses on real-time performance and memory efficiency
 * for the ARM Cortex-A9 processor. All functions are optimized for minimal overhead
 * and maximum responsiveness during gameplay.
 * 
 * @author [Daniel Grijalva]
 * @date [10/18/2025]
 * @version 1.0
 * 
 * @target_platform DE10-Standard FPGA Board (Cyclone V SoC)
 * @target_os Embedded Linux
 * @compiler ARM GCC
 * 
 * @dependencies
 * - game_state.h: Function declarations and type definitions
 * - address_map_arm.h: Memory-mapped I/O definitions
 * 
 * @note This implementation is specifically designed for the DE10-Standard board
 *       and assumes a 32-bit ARM architecture with memory-mapped peripherals.
 * 
 * @warning Thread safety is not implemented in this version. Use proper
 *          synchronization mechanisms if multiple threads access the game state.
 * 
 * @todo Add persistent storage functionality for high scores
 * @todo Implement thread-safe access mechanisms
 * @todo Add input validation for all setter functions
 */

#include "game_state.h"

#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

static game_state_t global_state;
static bool global_state_initialized = false;

 /**
  * @brief Initialize the game state structure with default values
  * 
  * This function initializes a game state structure with safe default values
  * suitable for starting a new game session. All counters are reset to their
  * initial state, and default game parameters are set according to the game
  * design specifications.
  * 
  * @param game_state Pointer to the game state structure to initialize
  * 
  * @pre game_state must be a valid pointer to a game_state_t structure
  * @post All game state variables are set to their default values
  * 
  * @details Initialization values:
  *          - score: 0 (no points earned yet)
  *          - level: 1 (start at first level)
  *          - lives: 3 (standard starting lives)
  *          - high_score: 0 (no previous high score)
  *          - current_time: 0 (no time elapsed in current session)
  *          - total_time: 0 (no total play time recorded)
  *          - current_level: 1 (start at first level)
  *          - total_levels: 10 (total levels available)
  *          - current_score: 0 (no score in current level)
  *          - total_score: 0 (no cumulative score)
  * 
  * @note This function should be called once at the start of each new game session.
  *       It does not perform any memory allocation or deallocation.
  * 
  * @warning This function does not validate the input pointer. Ensure the
  *          pointer is valid and points to a properly allocated game_state_t
  *          structure before calling this function.
  * 
  * @see game_state_update() for updating the state during gameplay
  * 
  * @example
  * @code
  * game_state_t game_state;
  * game_state_init(&game_state);
  * // Game state is now initialized and ready for use
  * @endcode
  */
void game_state_init(game_state_t *game_state) {
    if (!game_state) {
        return;
    }

    // Initialize scoring variables
    game_state->score = 0;           // Current session score
    game_state->current_score = 0;   // Current level score
    game_state->total_score = 0;     // Cumulative score
    game_state->high_score = game_state_load_high_score();  // Load from persistent storage

    // Initialize level progression variables
    game_state->level = 1;           // Current difficulty level
    game_state->current_level = 1;   // Current level in session
    game_state->total_levels = 10;   // Total available levels

    // Initialize player state variables
    game_state->lives = 3;           // Starting lives count

    // Initialize timing variables
    game_state->current_time = 0;    // Current session time
    game_state->total_time = 0;      // Total play time

    memcpy(&global_state, game_state, sizeof(game_state_t));
    global_state_initialized = true;
}

void game_state_update(game_state_t *game_state) {
    if (!game_state) {
        return;
    }

    game_state->current_time += 1;
    game_state->total_time += 1;

    if (game_state->current_score > game_state->high_score) {
        game_state->high_score = game_state->current_score;
        // Save high score to persistent storage when it changes
        game_state_save_high_score(game_state->high_score);
    }

    memcpy(&global_state, game_state, sizeof(game_state_t));
    global_state_initialized = true;
}

game_state_t *game_state_get(void) {
    if (!global_state_initialized) {
        return NULL;
    }
    return &global_state;
}

void game_state_set(game_state_t *game_state) {
    if (!game_state) {
        return;
    }

    memcpy(&global_state, game_state, sizeof(game_state_t));
    global_state_initialized = true;
}

int game_state_get_lives(void) {
    if (!global_state_initialized) {
        return 0;
    }
    return global_state.lives;
}

void game_state_set_lives(int lives) {
    if (!global_state_initialized) {
        return;
    }

    if (lives < 0) {
        lives = 0;
    }
    global_state.lives = lives;
}

int game_state_get_high_score(void) {
    if (!global_state_initialized) {
        return 0;
    }
    return global_state.high_score;
}

void game_state_set_high_score(int high_score) {
    if (!global_state_initialized) {
        return;
    }

    if (high_score < 0) {
        high_score = 0;
    }
    global_state.high_score = high_score;
}

int game_state_get_current_time(void) {
    if (!global_state_initialized) {
        return 0;
    }
    return global_state.current_time;
}

void game_state_set_current_time(int current_time) {
    if (!global_state_initialized) {
        return;
    }

    if (current_time < 0) {
        current_time = 0;
    }
    global_state.current_time = current_time;
}

#define HIGH_SCORE_FILE "/tmp/flappy_bird_highscore.txt"

int game_state_load_high_score(void) {
    FILE *file = fopen(HIGH_SCORE_FILE, "r");
    if (!file) {
        // File doesn't exist or can't be opened - return 0 (no high score)
        return 0;
    }

    int high_score = 0;
    if (fscanf(file, "%d", &high_score) != 1) {
        high_score = 0;
    }
    fclose(file);

    // Ensure high score is non-negative
    if (high_score < 0) {
        high_score = 0;
    }

    return high_score;
}

int game_state_save_high_score(int high_score) {
    if (high_score < 0) {
        return -1;
    }

    FILE *file = fopen(HIGH_SCORE_FILE, "w");
    if (!file) {
        return -1;
    }

    int result = fprintf(file, "%d\n", high_score);
    fclose(file);

    if (result < 0) {
        return -1;
    }

    return 0;
}