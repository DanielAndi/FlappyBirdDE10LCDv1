/**
 * @file gameplay.c
 * @brief Implementation of main gameplay loop and game logic
 * 
 * This file implements the main gameplay loop for the Flappy Bird game.
 * It handles game state management, object updates, collision detection,
 * scoring, and rendering.
 * 
 * @author Daniel Grijalva
 * @date 2025
 * @version 1.0
 */

#include "gameplay.h"

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

#include "display/lcd_driver.h"
#include "display/sprites.h"
#include "game/collision.h"
#include "game/game_objects.h"
#include "game/game_state.h"
#include "game/physics.h"
#include "hardware/fpga_peripherals.h"

/** Target frame time in seconds (60 FPS) */
#define FRAME_TIME_SECONDS (1.0f / 60.0f)

/** Frame delay in microseconds */
#define FRAME_TIME_USEC 16000U

/** Gravity acceleration (pixels per second squared) */
#define GRAVITY_ACCELERATION 2000.0f

/** Jump impulse (pixels per second, negative for upward) */
#define JUMP_IMPULSE -250.0f

/** Bird starting X position */
#define BIRD_START_X 32U

/** Number of pipe pairs in the game */
#define PIPE_COUNT 3U

/** Spacing between pipe pairs */
#define PIPE_SPACING 48.0f

/** Pipe movement speed (pixels per second) */
#define PIPE_SPEED 55.0f

/** Height of gap between top and bottom pipes */
#define PIPE_GAP_HEIGHT 32.0f

/** Margin from screen edges for pipe placement */
#define PIPE_MARGIN 8.0f

/**
 * @brief Game mode enumeration
 * 
 * Represents the current state of the game.
 */
typedef enum {
    GAME_MODE_READY = 0,    /**< Ready state (waiting to start) */
    GAME_MODE_PLAYING,      /**< Active gameplay */
    GAME_MODE_GAME_OVER     /**< Game over state */
} game_mode_t;

static float next_gap_y(uint32_t *rng_state) {
    if (!rng_state) {
        return PIPE_MARGIN;
    }

    *rng_state = (*rng_state * 1103515245U) + 12345U;
    const float max_gap_y = (float)LCD_HEIGHT - PIPE_GAP_HEIGHT - PIPE_MARGIN;
    const float range = max_gap_y - PIPE_MARGIN;
    const float normalized = (float)(*rng_state & 0x7FFFU) / 32767.0f;
    const float raw_gap = PIPE_MARGIN + (normalized * range);
    const float quantized = (float)((int)(raw_gap / 8.0f)) * 8.0f;
    if (quantized < PIPE_MARGIN) {
        return PIPE_MARGIN;
    }
    if (quantized > max_gap_y) {
        return max_gap_y;
    }
    return quantized;
}

static void spawn_pipe(pipe_pair_t *pipe, float x, float gap_height, float speed,
                       uint32_t *rng_state) {
    if (!pipe) {
        return;
    }
    const float gap_y = next_gap_y(rng_state);
    pipe_pair_init(pipe, x, gap_y, gap_height, speed);
}

static void spawn_pipe_line(pipe_pair_t pipes[], bool scored[], size_t count,
                            float initial_x, float spacing, float gap_height,
                            float speed, uint32_t *rng_state) {
    if (!pipes || !scored) {
        return;
    }

    float current_x = initial_x;
    for (size_t i = 0U; i < count; ++i) {
        spawn_pipe(&pipes[i], current_x, gap_height, speed, rng_state);
        scored[i] = false;
        current_x += spacing;
    }
}

static void update_pipes(pipe_pair_t pipes[], bool scored[], size_t count,
                         float dt, float spacing, float gap_height, float speed,
                         uint32_t *rng_state) {
    if (!pipes || !scored || count == 0U) {
        return;
    }

    float max_x = pipes[0].x;
    for (size_t i = 1U; i < count; ++i) {
        if (pipes[i].x > max_x) {
            max_x = pipes[i].x;
        }
    }

    for (size_t i = 0U; i < count; ++i) {
        pipe_pair_update(&pipes[i], dt);
        if (pipes[i].x > max_x) {
            max_x = pipes[i].x;
        }
    }

    for (size_t i = 0U; i < count; ++i) {
        const float pipe_width = pipes[i].top_bounds.width;
        if ((pipes[i].x + pipe_width) < 0.0f) {
            max_x += spacing;
            spawn_pipe(&pipes[i], max_x, gap_height, speed, rng_state);
            scored[i] = false;
        }
    }
}

static void draw_pipe_section(graphics_context_t *gfx, uint32_t x, uint32_t start_y,
                              uint32_t height, const sprite_t *sprite) {
    if (!gfx || !sprite || sprite->height == 0U) {
        return;
    }

    uint32_t offset = 0U;
    while (offset < height) {
        const uint32_t draw_y = start_y + offset;
        if (draw_y >= LCD_HEIGHT) {
            break;
        }
        graphics_draw_sprite(gfx, x, draw_y, sprite);
        offset += sprite->height;
    }
}

static void draw_pipes(graphics_context_t *gfx, const pipe_pair_t pipes[],
                       size_t count, const sprite_t *pipe_sprite) {
    if (!gfx || !pipes || !pipe_sprite) {
        return;
    }

    for (size_t i = 0U; i < count; ++i) {
        const pipe_pair_t *pipe = &pipes[i];
        const uint32_t pipe_x = (uint32_t)(pipe->x + 0.5f);
        const uint32_t top_height = (uint32_t)(pipe->top_bounds.height + 0.5f);
        const uint32_t bottom_y = (uint32_t)(pipe->bottom_bounds.y + 0.5f);
        const uint32_t bottom_height =
            (uint32_t)(pipe->bottom_bounds.height + 0.5f);

        draw_pipe_section(gfx, pipe_x, 0U, top_height, pipe_sprite);
        draw_pipe_section(gfx, pipe_x, bottom_y, bottom_height, pipe_sprite);
    }
}

static bool check_pipe_collisions(const aabb_t *bird_bounds,
                                  const pipe_pair_t pipes[], size_t count) {
    if (!bird_bounds || !pipes) {
        return false;
    }

    for (size_t i = 0U; i < count; ++i) {
        if (collision_aabb_intersect(bird_bounds, &pipes[i].top_bounds) ||
            collision_aabb_intersect(bird_bounds, &pipes[i].bottom_bounds)) {
            return true;
        }
    }
    return false;
}

void gameplay_run(graphics_context_t *gfx,
                  input_state_t *input,
                  fpga_bridge_t *bridge,
                  volatile sig_atomic_t *should_exit) {
    if (!gfx || !input || !bridge) {
        return;
    }

    const sprite_t *bird = sprites_get_bird();
    const sprite_t *pipe_sprite = sprites_get_pipe();
    if (!bird || !pipe_sprite) {
        return;
    }

    // Initialize hex display for score
    fpga_hex_init(bridge);

    // Initialize game state
    game_state_t game_state;
    game_state_init(&game_state);
    game_state_set(&game_state);

    physics_body_t bird_body;
    physics_init(&bird_body);

    const float ground_y = (float)(LCD_HEIGHT - bird->height);
    const float ceiling_y = 0.0f;
    const float starting_y =
        (float)((LCD_HEIGHT / 2U) - (bird->height / 2U));
    bird_body.position = starting_y;

    bool was_jump_pressed = false;
    bool pipe_scored[PIPE_COUNT] = { false };
    pipe_pair_t pipes[PIPE_COUNT];
    uint32_t rng_state = 0x1U;
    game_mode_t mode = GAME_MODE_READY;
    
    // Display initial score of 0
    fpga_hex_display(bridge, 0U);

    aabb_t bird_bounds =
        collision_create((float)BIRD_START_X, bird_body.position,
                         (float)bird->width, (float)bird->height);

    spawn_pipe_line(pipes, pipe_scored, PIPE_COUNT,
                    (float)LCD_WIDTH + 20.0f, PIPE_SPACING, PIPE_GAP_HEIGHT,
                    PIPE_SPEED, &rng_state);

    while (!(should_exit && *should_exit)) {
        input_handler_poll(input, bridge);
        bool is_jump_pressed = input_handler_is_jump_pressed(input);
        if (is_jump_pressed && !was_jump_pressed) {
            if (mode == GAME_MODE_READY) {
                physics_reset(&bird_body);
                bird_body.position = starting_y;
                physics_apply_impulse(&bird_body, JUMP_IMPULSE);
                mode = GAME_MODE_PLAYING;
                // Reset score in game state
                game_state_t *state = game_state_get();
                if (state) {
                    state->score = 0;
                    state->current_score = 0;
                    game_state_set(state);
                }
                fpga_hex_display(bridge, 0U);  // Reset hex display to 0
                for (size_t i = 0U; i < PIPE_COUNT; ++i) {
                    pipe_scored[i] = false;
                }
                spawn_pipe_line(pipes, pipe_scored, PIPE_COUNT,
                                (float)LCD_WIDTH + 20.0f, PIPE_SPACING,
                                PIPE_GAP_HEIGHT, PIPE_SPEED, &rng_state);
            } else if (mode == GAME_MODE_PLAYING) {
                physics_apply_impulse(&bird_body, JUMP_IMPULSE);
            } else if (mode == GAME_MODE_GAME_OVER) {
                physics_reset(&bird_body);
                bird_body.position = starting_y;
                // Reset score in game state
                game_state_t *state = game_state_get();
                if (state) {
                    state->score = 0;
                    state->current_score = 0;
                    game_state_set(state);
                }
                fpga_hex_display(bridge, 0U);  // Reset hex display to 0
                spawn_pipe_line(pipes, pipe_scored, PIPE_COUNT,
                                (float)LCD_WIDTH + 20.0f, PIPE_SPACING,
                                PIPE_GAP_HEIGHT, PIPE_SPEED, &rng_state);
                mode = GAME_MODE_READY;
            }
        }
        was_jump_pressed = is_jump_pressed;

        if (mode == GAME_MODE_PLAYING) {
            physics_apply_gravity(&bird_body, GRAVITY_ACCELERATION);
            physics_update(&bird_body, FRAME_TIME_SECONDS);

            if (bird_body.position > ground_y) {
                bird_body.position = ground_y;
                bird_body.velocity = 0.0f;
                mode = GAME_MODE_GAME_OVER;
                // Update high score if current score is higher
                game_state_t *state = game_state_get();
                if (state) {
                    if (state->score > state->high_score) {
                        state->high_score = state->score;
                        state->current_score = state->score;
                        game_state_update(state);
                    }
                }
            } else if (bird_body.position < ceiling_y) {
                bird_body.position = ceiling_y;
                if (bird_body.velocity < 0.0f) {
                    bird_body.velocity = 0.0f;
                }
            }

            bird_bounds.x = (float)BIRD_START_X;
            bird_bounds.y = bird_body.position;

            update_pipes(pipes, pipe_scored, PIPE_COUNT, FRAME_TIME_SECONDS,
                         PIPE_SPACING, PIPE_GAP_HEIGHT, PIPE_SPEED,
                         &rng_state);

            if (check_pipe_collisions(&bird_bounds, pipes, PIPE_COUNT)) {
                mode = GAME_MODE_GAME_OVER;
                // Update high score if current score is higher
                game_state_t *state = game_state_get();
                if (state) {
                    if (state->score > state->high_score) {
                        state->high_score = state->score;
                        state->current_score = state->score;
                        game_state_update(state);
                    }
                }
            }

            for (size_t i = 0U; i < PIPE_COUNT; ++i) {
                const float pipe_right =
                    pipes[i].x + pipes[i].top_bounds.width;
                if (!pipe_scored[i] && pipe_right < (float)BIRD_START_X) {
                    // Update score in game state
                    game_state_t *state = game_state_get();
                    if (state) {
                        state->score++;
                        state->current_score = state->score;
                        // Update high score if needed
                        if (state->score > state->high_score) {
                            state->high_score = state->score;
                        }
                        game_state_update(state);
                        fpga_hex_display(bridge, (uint32_t)state->score);
                    }
                    pipe_scored[i] = true;
                }
            }
        } else {
            bird_body.velocity = 0.0f;
            if (mode == GAME_MODE_READY) {
                bird_body.position = starting_y;
            }
        }

        graphics_clear(gfx);

        draw_pipes(gfx, pipes, PIPE_COUNT, pipe_sprite);

        graphics_draw_sprite(gfx, BIRD_START_X,
                             (uint32_t)(bird_body.position + 0.5f), bird);

        // Get current game state for display
        game_state_t *state = game_state_get();
        uint32_t current_score = 0U;
        int high_score = 0;
        if (state) {
            current_score = (uint32_t)state->score;
            high_score = state->high_score;
        }

        if (mode == GAME_MODE_READY) {
            graphics_draw_text(gfx, 10U, 8U, "PRESS!");
            graphics_draw_text(gfx, 10U, 24U, "PRESS!");
            // Display high score on ready screen
            if (high_score > 0) {
                graphics_draw_text(gfx, 10U, 40U, "HI:");
                graphics_draw_number(gfx, 40U, 40U, (uint32_t)high_score);
            }
        } else if (mode == GAME_MODE_GAME_OVER) {
            graphics_draw_text(gfx, 10U, 8U, "LOSE!");
            graphics_draw_text(gfx, 10U, 24U, "PRESS!");
            // Display score and high score on game over screen
            graphics_draw_text(gfx, 10U, 40U, "SCORE:");
            graphics_draw_number(gfx, 70U, 40U, current_score);
            graphics_draw_text(gfx, 10U, 48U, "HI:");
            graphics_draw_number(gfx, 40U, 48U, (uint32_t)high_score);
        } else {
            // Display current score during gameplay (top right)
            graphics_draw_text(gfx, 80U, 0U, "S:");
            graphics_draw_number(gfx, 100U, 0U, current_score);
        }

        graphics_present(gfx);

        if ((input->switches & 0x1U) != 0U) {
            break;
        }
        if (should_exit && *should_exit) {
            break;
        }

        usleep(FRAME_TIME_USEC);
    }
}

