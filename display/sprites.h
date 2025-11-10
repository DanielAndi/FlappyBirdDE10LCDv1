/**
 * @file sprites.h
 * @brief Sprite data structures and accessor functions
 * 
 * This module provides sprite definitions and accessor functions for game sprites.
 * Sprites are stored as bitmap data with width and height information.
 * 
 * @author Daniel Grijalva
 * @date 2025
 * @version 1.0
 */

#ifndef SPRITES_H
#define SPRITES_H

#include <stdint.h>

/**
 * @brief Sprite structure containing bitmap data
 * 
 * Represents a 2D bitmap sprite with width, height, and pixel data.
 * The bitmap is stored as a 1-bit per pixel array (row-major order).
 */
typedef struct {
    uint32_t width;         /**< Sprite width in pixels */
    uint32_t height;        /**< Sprite height in pixels */
    const uint8_t *bitmap;  /**< Pointer to bitmap data (1-bit per pixel) */
} sprite_t;

/**
 * @brief Get the bird sprite
 * 
 * Returns a pointer to the bird sprite structure.
 * 
 * @return Pointer to bird sprite (valid for the lifetime of the program)
 */
const sprite_t *sprites_get_bird(void);

/**
 * @brief Get the pipe sprite
 * 
 * Returns a pointer to the pipe sprite structure.
 * 
 * @return Pointer to pipe sprite (valid for the lifetime of the program)
 */
const sprite_t *sprites_get_pipe(void);

#endif /* SPRITES_H */

