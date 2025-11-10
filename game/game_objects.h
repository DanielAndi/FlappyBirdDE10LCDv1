/**
 * @file game_objects.h
 * @brief Game object definitions and management for Flappy Bird
 * 
 * This module defines the main game entities (bird and pipes) and provides
 * functions to initialize, update, and manipulate them. Game objects combine
 * physics bodies with collision bounds for complete game entity management.
 * 
 * @author Daniel Grijalva
 * @date 2025
 * @version 1.0
 */

#ifndef GAME_OBJECTS_H
#define GAME_OBJECTS_H

#include "collision.h"
#include "physics.h"

/**
 * @brief Bird game object structure
 * 
 * Represents the player-controlled bird character. Combines physics
 * simulation with collision detection bounds.
 */
typedef struct {
    physics_body_t body;  /**< Physics body for movement simulation */
    aabb_t bounds;        /**< Collision bounding box */
} bird_t;

/**
 * @brief Pipe pair game object structure
 * 
 * Represents a pair of pipes (top and bottom) that form an obstacle.
 * The pipes have a gap between them that the bird must navigate through.
 */
typedef struct {
    float x;              /**< X position of the pipe pair */
    float gap_y;          /**< Y position of the top of the gap */
    float gap_height;     /**< Height of the gap between pipes */
    aabb_t top_bounds;    /**< Collision bounds for top pipe */
    aabb_t bottom_bounds; /**< Collision bounds for bottom pipe */
    float speed;          /**< Horizontal movement speed (pixels per second) */
} pipe_pair_t;

/**
 * @brief Initialize a bird object
 * 
 * Sets up a bird at the specified position with default physics state.
 * 
 * @param bird Pointer to bird structure to initialize
 * @param x Initial X position
 * @param y Initial Y position
 * 
 * @note This function performs null pointer checking
 */
void bird_init(bird_t *bird, float x, float y);

/**
 * @brief Update bird physics and collision bounds
 * 
 * Updates the bird's physics state and synchronizes the collision
 * bounds with the current position.
 * 
 * @param bird Pointer to bird structure to update
 * @param dt Time step in seconds
 * 
 * @note This function performs null pointer checking
 * @note Gravity should be applied separately before calling this function
 */
void bird_update(bird_t *bird, float dt);

/**
 * @brief Apply a flap (jump) impulse to the bird
 * 
 * Applies an upward impulse to make the bird "flap" and gain altitude.
 * 
 * @param bird Pointer to bird structure
 * @param impulse Impulse value (typically negative for upward movement)
 * 
 * @note This function performs null pointer checking
 */
void bird_flap(bird_t *bird, float impulse);

/**
 * @brief Initialize a pipe pair object
 * 
 * Sets up a pipe pair at the specified position with a gap.
 * 
 * @param pipe Pointer to pipe pair structure to initialize
 * @param x Initial X position
 * @param gap_y Y position of the top of the gap
 * @param gap_height Height of the gap between top and bottom pipes
 * @param speed Horizontal movement speed (pixels per second)
 * 
 * @note This function performs null pointer checking
 */
void pipe_pair_init(pipe_pair_t *pipe, float x, float gap_y, float gap_height, float speed);

/**
 * @brief Update pipe pair position
 * 
 * Moves the pipe pair horizontally and updates collision bounds.
 * 
 * @param pipe Pointer to pipe pair structure to update
 * @param dt Time step in seconds
 * 
 * @note This function performs null pointer checking
 */
void pipe_pair_update(pipe_pair_t *pipe, float dt);

/**
 * @brief Reset pipe pair to a new position
 * 
 * Moves the pipe pair to a new X position and updates the gap position.
 * 
 * @param pipe Pointer to pipe pair structure to reset
 * @param x New X position
 * @param gap_y New Y position of the top of the gap
 * 
 * @note This function performs null pointer checking
 */
void pipe_pair_reset(pipe_pair_t *pipe, float x, float gap_y);

#endif /* GAME_OBJECTS_H */

