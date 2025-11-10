/**
 * @file graphics.c
 * @brief Implementation of graphics rendering functions
 * 
 * This file implements the graphics functions declared in graphics.h.
 * It provides pixel, sprite, and text rendering on the LCD display.
 * 
 * @author Daniel Grijalva
 * @date 2025
 * @version 1.0
 */

#include "graphics.h"

#include <ctype.h>
#include <stddef.h>

typedef struct {
    char character;
    uint8_t rows[7];
} glyph_t;

static const glyph_t FONT_5X7[] = {
    { '!', { 0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x04 } },
    { 'B', { 0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E } },
    { 'E', { 0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F } },
    { 'H', { 0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11 } },
    { 'L', { 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F } },
    { 'N', { 0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11 } },
    { 'O', { 0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E } },
    { 'P', { 0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10 } },
    { 'R', { 0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11 } },
    { 'S', { 0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E } },
    { 'T', { 0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04 } },
    { 'U', { 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E } },
};

static const glyph_t *lookup_glyph(char character) {
    char normalized = (char)toupper((unsigned char)character);
    for (size_t i = 0; i < (sizeof(FONT_5X7) / sizeof(FONT_5X7[0])); ++i) {
        if (FONT_5X7[i].character == normalized) {
            return &FONT_5X7[i];
        }
    }
    return NULL;
}

void graphics_init(graphics_context_t *ctx) {
    if (!ctx) {
        return;
    }

    lcd_driver_init();
    lcd_driver_clear();

    ctx->draw_color = 1U;
}

void graphics_clear(graphics_context_t *ctx) {
    (void)ctx;
    lcd_driver_clear();
}

void graphics_draw_pixel(graphics_context_t *ctx, uint32_t x, uint32_t y) {
    if (!ctx) {
        return;
    }

    if (x >= LCD_WIDTH || y >= LCD_HEIGHT) {
        return;
    }

    const uint32_t mapped_x = (LCD_WIDTH - 1U) - x;
    lcd_driver_draw_pixel(mapped_x, y, ctx->draw_color);
}

void graphics_draw_sprite(graphics_context_t *ctx, uint32_t x, uint32_t y,
                          const sprite_t *sprite) {
    if (!ctx || !sprite || !sprite->bitmap) {
        return;
    }

    for (uint32_t row = 0U; row < sprite->height; ++row) {
        for (uint32_t col = 0U; col < sprite->width; ++col) {
            const uint32_t index = row * sprite->width + col;
            if (sprite->bitmap[index] != 0U) {
                graphics_draw_pixel(ctx, x + col, y + row);
            }
        }
    }
}

void graphics_draw_text(graphics_context_t *ctx, uint32_t x, uint32_t y,
                        const char *text) {
    if (!ctx || !text) {
        return;
    }

    uint32_t cursor_x = x;
    uint32_t cursor_y = y;

    for (const char *ch = text; *ch != '\0'; ++ch) {
        if (*ch == '\n') {
            cursor_y += 8U;
            cursor_x = x;
            continue;
        }

        if (*ch == ' ') {
            cursor_x += 6U;
            continue;
        }

        const glyph_t *glyph = lookup_glyph(*ch);
        if (!glyph) {
            cursor_x += 6U;
            continue;
        }

        for (uint32_t row = 0U; row < 7U; ++row) {
            uint8_t row_bits = glyph->rows[row];
            for (uint32_t col = 0U; col < 5U; ++col) {
                if (((row_bits >> (4U - col)) & 0x1U) != 0U) {
                    graphics_draw_pixel(ctx, cursor_x + col, cursor_y + row);
                }
            }
        }

        cursor_x += 6U;
    }
}

void graphics_present(graphics_context_t *ctx) {
    (void)ctx;
    lcd_driver_flush();
}

void graphics_shutdown(graphics_context_t *ctx) {
    (void)ctx;
    lcd_driver_shutdown();
}

void graphics_draw_startup_screen(graphics_context_t *ctx) {
    if (!ctx) {
        return;
    }

    graphics_clear(ctx);
    graphics_draw_text(ctx, 16U, 16U, "HELLO!");
    graphics_draw_text(ctx, 8U, 32U, "PRESS BUTTON");
    graphics_present(ctx);
}

