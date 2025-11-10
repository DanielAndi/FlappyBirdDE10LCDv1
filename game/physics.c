/**
 * @file physics.c
 * @brief Implementation of physics engine functions
 * 
 * This file implements the physics engine functions declared in physics.h.
 * It provides simple Euler integration for 2D game physics.
 * 
 * @author Daniel Grijalva
 * @date 2025
 * @version 1.0
 */

#include "physics.h"

void physics_init(physics_body_t *body) {
    if (!body) {
        return;
    }

    body->position = 0.0f;
    body->velocity = 0.0f;
    body->acceleration = 0.0f;
}

void physics_apply_gravity(physics_body_t *body, float gravity) {
    if (!body) {
        return;
    }

    body->acceleration += gravity;
}

void physics_apply_impulse(physics_body_t *body, float impulse) {
    if (!body) {
        return;
    }

    body->velocity += impulse;
}

void physics_update(physics_body_t *body, float dt) {
    if (!body) {
        return;
    }

    body->velocity += body->acceleration * dt;
    body->position += body->velocity * dt;
    body->acceleration = 0.0f;
}

void physics_reset(physics_body_t *body) {
    physics_init(body);
}

