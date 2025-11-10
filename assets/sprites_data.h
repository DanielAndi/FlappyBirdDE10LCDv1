/**
 * @file sprites_data.h
 * @brief Placeholder sprite bitmaps for development and testing.
 */

#ifndef SPRITES_DATA_H
#define SPRITES_DATA_H

#include <stdint.h>

#define SPRITE_BIRD_WIDTH 4U
#define SPRITE_BIRD_HEIGHT 4U
static const uint8_t SPRITE_BIRD_BITMAP[SPRITE_BIRD_WIDTH * SPRITE_BIRD_HEIGHT] = {
    0U, 1U, 1U, 0U,
    0U, 1U, 1U, 1U,
    1U, 1U, 1U, 1U,
    0U, 1U, 1U, 0U,
};

#define SPRITE_PIPE_WIDTH 4U
#define SPRITE_PIPE_HEIGHT 8U
static const uint8_t SPRITE_PIPE_BITMAP[SPRITE_PIPE_WIDTH * SPRITE_PIPE_HEIGHT] = {
    1U, 1U, 1U, 1U,
    1U, 0U, 0U, 1U,
    1U, 0U, 0U, 1U,
    1U, 0U, 0U, 1U,
    1U, 0U, 0U, 1U,
    1U, 0U, 0U, 1U,
    1U, 1U, 1U, 1U,
    1U, 1U, 1U, 1U,
};

#endif /* SPRITES_DATA_H */

