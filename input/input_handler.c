/**
 * @file input_handler.c
 * @brief Implementation of input handler functions
 * 
 * This file implements the input handler functions declared in input_handler.h.
 * It provides polling and waiting functionality for button and switch inputs
 * from the DE10-Standard board's FPGA peripherals.
 * 
 * @author Daniel Grijalva
 * @date 2025
 * @version 1.0
 */

#include "input_handler.h"

#include <signal.h>
#include <unistd.h>

#include "hardware/fpga_peripherals.h"

/** Bitmask for KEY0 (jump button) */
#define JUMP_KEY_MASK 0x1U

/** Polling delay in microseconds (approximately 16ms for ~60Hz polling) */
#define BUTTON_POLL_USEC 16000U

int input_handler_init(input_state_t *state, fpga_bridge_t *bridge) {
    if (!state || !bridge) {
        return -1;
    }

    if (fpga_keys_init(bridge) != 0) {
        return -1;
    }

    fpga_keys_clear_edges(bridge);
    fpga_switches_init(bridge);

    input_handler_poll(state, bridge);
    return 0;
}

void input_handler_poll(input_state_t *state, fpga_bridge_t *bridge) {
    if (!state || !bridge) {
        return;
    }

    state->keys = fpga_keys_read(bridge);
    state->switches = fpga_switches_read(bridge);
}

bool input_handler_is_jump_pressed(const input_state_t *state) {
    if (!state) {
        return false;
    }

    // Keys are active-low; bit clears to 0 when pressed.
    return (state->keys & JUMP_KEY_MASK) == 0U;
}

void input_handler_wait_for_button_release(input_state_t *state, fpga_bridge_t *bridge,
                                           volatile sig_atomic_t *should_exit) {
    if (!state || !bridge) {
        return;
    }

    while (!(should_exit && *should_exit)) {
        input_handler_poll(state, bridge);
        if (!input_handler_is_jump_pressed(state)) {
            break;
        }
        if (should_exit && *should_exit) {
            break;
        }
        usleep(BUTTON_POLL_USEC);
    }
}

void input_handler_wait_for_button_press(input_state_t *state, fpga_bridge_t *bridge,
                                         volatile sig_atomic_t *should_exit) {
    if (!state || !bridge) {
        return;
    }

    input_handler_wait_for_button_release(state, bridge, should_exit);

    while (!(should_exit && *should_exit)) {
        input_handler_poll(state, bridge);
        if (input_handler_is_jump_pressed(state)) {
            break;
        }
        if (should_exit && *should_exit) {
            break;
        }
        usleep(BUTTON_POLL_USEC);
    }
}

