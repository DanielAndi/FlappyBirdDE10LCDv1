/**
 * @file sprites.c
 * @brief Implementation of sprite accessor functions
 * 
 * This file implements the sprite accessor functions declared in sprites.h.
 * It provides access to sprite data stored in the assets/sprites_data.h file.
 * 
 * @author Daniel Grijalva
 * @date 2025
 * @version 1.0
 */

#include "sprites.h"
#include "assets/sprites_data.h"

static const sprite_t bird_sprite = {
    .width = SPRITE_BIRD_WIDTH,
    .height = SPRITE_BIRD_HEIGHT,
    .bitmap = SPRITE_BIRD_BITMAP,
};

static const sprite_t pipe_sprite = {
    .width = SPRITE_PIPE_WIDTH,
    .height = SPRITE_PIPE_HEIGHT,
    .bitmap = SPRITE_PIPE_BITMAP,
};

const sprite_t *sprites_get_bird(void) {
    return &bird_sprite;
}

const sprite_t *sprites_get_pipe(void) {
    return &pipe_sprite;
}

