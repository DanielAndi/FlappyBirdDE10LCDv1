/**
 * @file graphics.h
 * @brief High-level graphics rendering API for game objects
 * 
 * This module provides a high-level graphics API built on top of the LCD driver.
 * It supports drawing pixels, sprites, and text on the LCD display. The graphics
 * context maintains drawing state and provides a simple interface for rendering.
 * 
 * @author Daniel Grijalva
 * @date 2025
 * @version 1.0
 * 
 * Coordinate System:
 * ------------------
 * - Origin (0, 0) is at the top-left corner
 * - X increases to the right (0 to LCD_WIDTH-1)
 * - Y increases downward (0 to LCD_HEIGHT-1)
 * - Display resolution: 128x64 pixels
 */

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h>

#include "lcd_driver.h"
#include "sprites.h"

/**
 * @brief Graphics context structure
 * 
 * Maintains the current drawing state and color for graphics operations.
 */
typedef struct {
    uint8_t draw_color;  /**< Current draw color (0 = off, 1 = on) */
} graphics_context_t;

/**
 * @brief Initialize the graphics subsystem
 * 
 * Initializes the LCD driver and graphics context. This must be called
 * before any other graphics functions.
 * 
 * @param ctx Pointer to graphics context to initialize
 * 
 * @note This function performs null pointer checking
 */
void graphics_init(graphics_context_t *ctx);

/**
 * @brief Clear the entire display
 * 
 * Clears all pixels on the LCD display to the background color.
 * 
 * @param ctx Pointer to graphics context
 * 
 * @note This function performs null pointer checking
 */
void graphics_clear(graphics_context_t *ctx);

/**
 * @brief Draw a single pixel on the display
 * 
 * @param ctx Pointer to graphics context
 * @param x X coordinate of the pixel (0 to LCD_WIDTH-1)
 * @param y Y coordinate of the pixel (0 to LCD_HEIGHT-1)
 * 
 * @note This function performs null pointer checking
 * @note Coordinates are clipped to display bounds
 */
void graphics_draw_pixel(graphics_context_t *ctx, uint32_t x, uint32_t y);

/**
 * @brief Draw a sprite on the display
 * 
 * Draws a sprite bitmap at the specified position using the current draw color.
 * 
 * @param ctx Pointer to graphics context
 * @param x X coordinate of the top-left corner of the sprite
 * @param y Y coordinate of the top-left corner of the sprite
 * @param sprite Pointer to sprite structure containing bitmap data
 * 
 * @note This function performs null pointer checking
 * @note Sprites are clipped to display bounds
 */
void graphics_draw_sprite(graphics_context_t *ctx, uint32_t x, uint32_t y,
                          const sprite_t *sprite);

/**
 * @brief Draw text on the display
 * 
 * Draws a string of text at the specified position using a built-in 5x7 font.
 * Only a limited set of characters are supported (see implementation).
 * 
 * @param ctx Pointer to graphics context
 * @param x X coordinate of the top-left corner of the text
 * @param y Y coordinate of the top-left corner of the text
 * @param text Null-terminated string to draw
 * 
 * @note This function performs null pointer checking
 * @note Only uppercase letters and limited punctuation are supported
 * @note Text is clipped to display bounds
 */
void graphics_draw_text(graphics_context_t *ctx, uint32_t x, uint32_t y,
                        const char *text);

/**
 * @brief Present the rendered frame to the display
 * 
 * Flushes the internal frame buffer to the LCD display. This should be
 * called after drawing operations to update the visible display.
 * 
 * @param ctx Pointer to graphics context
 * 
 * @note This function performs null pointer checking
 */
void graphics_present(graphics_context_t *ctx);

/**
 * @brief Shutdown the graphics subsystem
 * 
 * Cleans up the graphics subsystem and LCD driver. This should be called
 * before program exit.
 * 
 * @param ctx Pointer to graphics context
 * 
 * @note This function performs null pointer checking
 */
void graphics_shutdown(graphics_context_t *ctx);

/**
 * @brief Draw the startup/welcome screen
 * 
 * Draws a welcome screen with game title and instructions.
 * This is typically displayed when the game starts.
 * 
 * @param ctx Pointer to graphics context
 * 
 * @note This function performs null pointer checking
 */
void graphics_draw_startup_screen(graphics_context_t *ctx);

#endif /* GRAPHICS_H */