/**
 * @file game_objects.c
 * @brief Implementation of game object functions
 * 
 * This file implements the game object functions declared in game_objects.h.
 * It provides initialization, update, and manipulation functions for bird
 * and pipe pair game entities.
 * 
 * @author Daniel Grijalva
 * @date 2025
 * @version 1.0
 */

#include "game_objects.h"
#include "display/lcd_driver.h"

/** Bird sprite width in pixels */
#define BIRD_WIDTH 4.0f

/** Bird sprite height in pixels */
#define BIRD_HEIGHT 4.0f

/** Pipe sprite width in pixels */
#define PIPE_WIDTH 4.0f

void bird_init(bird_t *bird, float x, float y) {
    if (!bird) {
        return;
    }

    physics_init(&bird->body);
    bird->body.position = y;
    bird->bounds = collision_create(x, y, BIRD_WIDTH, BIRD_HEIGHT);
}

void bird_update(bird_t *bird, float dt) {
    if (!bird) {
        return;
    }

    physics_apply_gravity(&bird->body, 0.0f);
    physics_update(&bird->body, dt);

    bird->bounds.y = bird->body.position;
}

void bird_flap(bird_t *bird, float impulse) {
    if (!bird) {
        return;
    }

    physics_apply_impulse(&bird->body, impulse);
}

void pipe_pair_init(pipe_pair_t *pipe, float x, float gap_y, float gap_height, float speed) {
    if (!pipe) {
        return;
    }

    pipe->x = x;
    pipe->gap_y = gap_y;
    pipe->gap_height = gap_height;
    pipe->speed = speed;

    pipe->top_bounds = collision_create(x, 0.0f, PIPE_WIDTH, gap_y);
    const float bottom_y = gap_y + gap_height;
    pipe->bottom_bounds = collision_create(x, bottom_y, PIPE_WIDTH, (float)LCD_HEIGHT - bottom_y);
}

void pipe_pair_update(pipe_pair_t *pipe, float dt) {
    if (!pipe) {
        return;
    }

    pipe->x -= pipe->speed * dt;
    pipe->top_bounds.x = pipe->x;
    pipe->bottom_bounds.x = pipe->x;
}

void pipe_pair_reset(pipe_pair_t *pipe, float x, float gap_y) {
    if (!pipe) {
        return;
    }

    pipe->x = x;
    pipe->gap_y = gap_y;

    pipe->top_bounds.x = x;
    pipe->top_bounds.height = gap_y;

    const float bottom_y = gap_y + pipe->gap_height;
    pipe->bottom_bounds.x = x;
    pipe->bottom_bounds.y = bottom_y;
    pipe->bottom_bounds.height = (float)LCD_HEIGHT - bottom_y;
}

