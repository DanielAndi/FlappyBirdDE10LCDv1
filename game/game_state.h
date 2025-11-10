/**
 * @file game_state.h
 * @brief Game state management module for Flappy Bird on DE10-Standard board
 * 
 * This module provides a comprehensive interface for managing the game state
 * in the Flappy Bird implementation running on the DE10-Standard FPGA board
 * with embedded Linux. It handles all game-related state variables including
 * scoring, level progression, timing, and player statistics.
 * 
 * @details The game state is designed to be persistent across game sessions
 * and provides thread-safe access to game variables. All functions are
 * optimized for real-time performance on the ARM Cortex-A9 processor.
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
 * - address_map_arm.h: Memory-mapped I/O definitions
 * - io_map.h: I/O port mapping definitions
 * 
 * @note This module is specifically designed for the DE10-Standard board
 *       and assumes a 32-bit ARM architecture with memory-mapped peripherals.
 * 
 * @warning Thread safety is not guaranteed for concurrent access to the same
 *          game state structure instance. Use proper synchronization if
 *          multiple threads access the game state.
 */

#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "hardware/address_map_arm.h"

/**
 * @def GAME_STATE_BASE
 * @brief Base memory address for game state storage
 * 
 * This address is used for memory-mapped I/O operations on the DE10-Standard
 * board. The game state data can be stored in dedicated memory regions or
 * shared memory areas accessible by the ARM processor.
 * 
 * @note This address should be aligned according to the memory controller
 *       requirements of the DE10-Standard board.
 */
#define GAME_STATE_BASE 0x00000000

/**
 * @struct game_state_t
 * @brief Complete game state structure containing all game variables
 * 
 * This structure encapsulates all game-related state information including
 * scoring, progression, timing, and player statistics. It is designed to
 * provide a centralized state management system for the Flappy Bird game.
 * 
 * @details All integer values are signed 32-bit integers to ensure
 *          compatibility with the ARM Cortex-A9 processor and to provide
 *          sufficient range for game statistics.
 * 
 * @note Structure size: 40 bytes (10 integers × 4 bytes each)
 * @note Memory alignment: 4-byte aligned for optimal ARM performance
 */
typedef struct {
    int score;           /**< Current game score (points earned in current session) */
    int level;           /**< Current difficulty level (1-based indexing) */
    int lives;           /**< Number of remaining lives/attempts */
    int high_score;      /**< Highest score achieved across all sessions */
    int current_time;    /**< Current session time in milliseconds */
    int total_time;      /**< Total time played across all sessions in milliseconds */
    int current_level;   /**< Current level within the current game session */
    int total_levels;    /**< Total number of levels available in the game */
    int current_score;   /**< Current score within the current level */
    int total_score;     /**< Total cumulative score across all sessions */
} game_state_t;

/**
 * @name Game State Management Functions
 * @{
 */

/**
 * @brief Initialize the game state structure with default values
 * 
 * This function initializes a game state structure with safe default values
 * suitable for starting a new game session. All counters are reset to their
 * initial state, and default game parameters are set.
 * 
 * @param game_state Pointer to the game state structure to initialize
 * 
 * @pre game_state must be a valid pointer to a game_state_t structure
 * @post All game state variables are set to their default values
 * 
 * @note Default values:
 *       - score: 0
 *       - level: 1
 *       - lives: 3
 *       - high_score: 0 (or loaded from persistent storage)
 *       - current_time: 0
 *       - total_time: 0 (or loaded from persistent storage)
 *       - current_level: 1
 *       - total_levels: 10 (configurable)
 *       - current_score: 0
 *       - total_score: 0
 * 
 * @warning This function does not validate the input pointer. Ensure the
 *          pointer is valid before calling this function.
 * 
 * @see game_state_update() for updating the state during gameplay
 */
void game_state_init(game_state_t *game_state);

/**
 * @brief Update the game state based on current game conditions
 * 
 * This function performs periodic updates to the game state, including
 * time increments, score calculations, and level progression checks.
 * It should be called regularly during the game loop to maintain
 * accurate state information.
 * 
 * @param game_state Pointer to the game state structure to update
 * 
 * @pre game_state must be a valid pointer to an initialized game_state_t structure
 * @post Game state is updated with current game conditions
 * 
 * @details This function typically handles:
 *          - Time increment calculations
 *          - Score updates based on game events
 *          - Level progression logic
 *          - Life count management
 *          - High score tracking
 * 
 * @note This function is designed to be called from the main game loop
 *       with appropriate timing intervals for smooth gameplay.
 * 
 * @see game_state_init() for initial state setup
 */
void game_state_update(game_state_t *game_state);

/**
 * @brief Retrieve a pointer to the current global game state
 * 
 * This function returns a pointer to the current global game state instance.
 * It provides thread-safe access to the game state for read operations.
 * 
 * @return Pointer to the current game state structure, or NULL if not initialized
 * 
 * @pre Game state must be initialized before calling this function
 * @post Returns valid pointer to game state structure
 * 
 * @note The returned pointer should not be modified directly. Use the
 *       appropriate setter functions for state modifications.
 * 
 * @warning The returned pointer becomes invalid if the game state is
 *          reinitialized. Always check for NULL return value.
 * 
 * @see game_state_set() for modifying the game state
 */
game_state_t *game_state_get(void);

/**
 * @brief Set the entire game state from a provided structure
 * 
 * This function copies the contents of a provided game state structure
 * to the current global game state. It performs a complete state replacement.
 * 
 * @param game_state Pointer to the game state structure to copy from
 * 
 * @pre game_state must be a valid pointer to a game_state_t structure
 * @post Current game state is replaced with the provided state
 * 
 * @note This function performs a deep copy of all state variables.
 *       Use this function for state restoration or game state loading.
 * 
 * @warning This function overwrites the entire current game state.
 *          Ensure the provided state is valid and complete.
 * 
 * @see game_state_get() for retrieving the current state
 */
void game_state_set(game_state_t *game_state);

/**
 * @name Individual State Accessor Functions
 * @{
 */

/**
 * @brief Get the current number of lives remaining
 * 
 * @return Current number of lives (non-negative integer)
 * 
 * @pre Game state must be initialized
 * @post Returns current life count
 * 
 * @note Returns 0 if no lives remaining (game over condition)
 * 
 * @see game_state_set_lives() for modifying the life count
 */
int game_state_get_lives(void);

/**
 * @brief Set the number of lives remaining
 * 
 * @param lives New number of lives (must be non-negative)
 * 
 * @pre lives must be >= 0
 * @post Life count is updated to the specified value
 * 
 * @note Setting lives to 0 triggers game over condition
 * @note Typical life counts are 1-5 for normal gameplay
 * 
 * @see game_state_get_lives() for retrieving the life count
 */
void game_state_set_lives(int lives);

/**
 * @brief Get the current high score
 * 
 * @return Current high score value
 * 
 * @pre Game state must be initialized
 * @post Returns current high score
 * 
 * @note High score is persistent across game sessions
 * @note High score is automatically updated when current score exceeds it
 * 
 * @see game_state_set_high_score() for manually setting high score
 */
int game_state_get_high_score(void);

/**
 * @brief Set the high score value
 * 
 * @param high_score New high score value (must be non-negative)
 * 
 * @pre high_score must be >= 0
 * @post High score is updated to the specified value
 * 
 * @note This function should typically be called automatically when
 *       the current score exceeds the high score
 * 
 * @see game_state_get_high_score() for retrieving the high score
 */
void game_state_set_high_score(int high_score);

/**
 * @brief Get the current session time
 * 
 * @return Current session time in milliseconds
 * 
 * @pre Game state must be initialized
 * @post Returns current session time
 * 
 * @note Time is measured from the start of the current game session
 * @note Time increments are typically handled by game_state_update()
 * 
 * @see game_state_set_current_time() for manually setting the time
 */
int game_state_get_current_time(void);

/**
 * @brief Set the current session time
 * 
 * @param current_time New current time in milliseconds (must be non-negative)
 * 
 * @pre current_time must be >= 0
 * @post Current session time is updated to the specified value
 * 
 * @note This function is typically used for time synchronization or
 *       game state restoration
 * 
 * @see game_state_get_current_time() for retrieving the current time
 */
void game_state_set_current_time(int current_time);

/**
 * @}
 */

#endif // GAME_STATE_H      