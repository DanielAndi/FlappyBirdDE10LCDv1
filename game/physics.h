/**
 * @file physics.h
 * @brief Simplified physics engine for game objects
 * 
 * This module provides a simple physics simulation system for game objects.
 * It implements basic kinematic equations for position, velocity, and acceleration,
 * suitable for 2D game physics like the bird's movement in Flappy Bird.
 * 
 * @author Daniel Grijalva
 * @date 2025
 * @version 1.0
 * 
 * Physics Model:
 * --------------
 * The physics system uses a simple Euler integration method:
 * - Velocity is updated by: v = v + a * dt
 * - Position is updated by: p = p + v * dt
 * 
 * After each update, acceleration is reset to zero and must be reapplied
 * each frame (e.g., gravity).
 */

#ifndef PHYSICS_H
#define PHYSICS_H

/**
 * @brief Physics body structure representing a moving object
 * 
 * This structure stores the kinematic state of a physics object.
 * All values are in game units (typically pixels for position).
 */
typedef struct {
    float position;      /**< Current position (in game units, e.g., pixels) */
    float velocity;      /**< Current velocity (units per second) */
    float acceleration;  /**< Current acceleration (units per second squared) */
} physics_body_t;

/**
 * @brief Initialize a physics body with zero position, velocity, and acceleration
 * 
 * @param body Pointer to physics body structure to initialize
 * 
 * @note This function performs null pointer checking
 */
void physics_init(physics_body_t *body);

/**
 * @brief Apply a gravity force to a physics body
 * 
 * Adds the specified gravity value to the body's acceleration.
 * Gravity is typically negative (downward) in screen coordinates.
 * 
 * @param body Pointer to physics body structure
 * @param gravity Gravity value to apply (units per second squared)
 * 
 * @note Gravity is additive - multiple calls will accumulate
 * @note This function performs null pointer checking
 */
void physics_apply_gravity(physics_body_t *body, float gravity);

/**
 * @brief Apply an impulse (instantaneous velocity change) to a physics body
 * 
 * Adds the specified impulse value to the body's velocity.
 * This is typically used for jump actions.
 * 
 * @param body Pointer to physics body structure
 * @param impulse Impulse value to apply (units per second, typically negative for upward jumps)
 * 
 * @note Impulses are additive - multiple calls will accumulate
 * @note This function performs null pointer checking
 */
void physics_apply_impulse(physics_body_t *body, float impulse);

/**
 * @brief Update physics body state based on elapsed time
 * 
 * Integrates the physics equations over the specified time step:
 * - Updates velocity: v = v + a * dt
 * - Updates position: p = p + v * dt
 * - Resets acceleration to zero
 * 
 * @param body Pointer to physics body structure to update
 * @param dt Time step in seconds
 * 
 * @note Acceleration is reset to zero after update
 * @note This function performs null pointer checking
 */
void physics_update(physics_body_t *body, float dt);

/**
 * @brief Reset a physics body to initial state (zero position, velocity, acceleration)
 * 
 * @param body Pointer to physics body structure to reset
 * 
 * @note This is equivalent to calling physics_init()
 * @note This function performs null pointer checking
 */
void physics_reset(physics_body_t *body);

#endif /* PHYSICS_H */