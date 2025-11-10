#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../address_map_arm.h"
#include "../fpga_peripherals.h"
#include "../hps_fpga_bridge.h"

typedef struct {
    fpga_bridge_t bridge;
    uint8_t *memory;
} mock_bridge_ctx_t;

static bool mock_bridge_init(mock_bridge_ctx_t *ctx) {
    if (ctx == NULL) {
        return false;
    }

    memset(ctx, 0, sizeof(*ctx));

    ctx->memory = (uint8_t *)calloc(1u, LW_BRIDGE_SPAN);
    if (ctx->memory == NULL) {
        fprintf(stderr, "mock_bridge_init: failed to allocate %u bytes\n", (unsigned)LW_BRIDGE_SPAN);
        return false;
    }

    ctx->bridge.virtual_base = ctx->memory;
    ctx->bridge.mem_fd = 0;
    ctx->bridge.initialized = true;
    return true;
}

static void mock_bridge_destroy(mock_bridge_ctx_t *ctx) {
    if (ctx == NULL) {
        return;
    }

    free(ctx->memory);
    ctx->memory = NULL;
    ctx->bridge.virtual_base = NULL;
    ctx->bridge.mem_fd = -1;
    ctx->bridge.initialized = false;
}

static volatile uint32_t *mock_reg_ptr(mock_bridge_ctx_t *ctx, uint32_t base, uint32_t offset) {
    return (volatile uint32_t *)((uint8_t *)ctx->bridge.virtual_base + base + offset);
}

static uint32_t mock_reg_read(mock_bridge_ctx_t *ctx, uint32_t base, uint32_t offset) {
    return *mock_reg_ptr(ctx, base, offset);
}

static void mock_reg_write(mock_bridge_ctx_t *ctx, uint32_t base, uint32_t offset, uint32_t value) {
    *mock_reg_ptr(ctx, base, offset) = value;
}

#define TEST_ASSERT(condition, message)                                                   \
    do {                                                                                  \
        if (!(condition)) {                                                               \
            fprintf(stderr, "%s:%d: Assertion failed: %s\n", __func__, __LINE__, message);\
            return false;                                                                 \
        }                                                                                 \
    } while (0)

#define TEST_ASSERT_EQ_UINT(expected, actual, message)                                            \
    do {                                                                                          \
        uint32_t exp_val = (uint32_t)(expected);                                                  \
        uint32_t act_val = (uint32_t)(actual);                                                    \
        if (exp_val != act_val) {                                                                 \
            fprintf(stderr, "%s:%d: Assertion failed: expected 0x%08X but got 0x%08X (%s)\n",     \
                    __func__, __LINE__, exp_val, act_val, message);                               \
            return false;                                                                         \
        }                                                                                         \
    } while (0)

static bool test_fpga_peripheral_read_write_basic(void) {
    mock_bridge_ctx_t ctx;
    TEST_ASSERT(mock_bridge_init(&ctx), "mock bridge should initialize");

    const uint32_t expected = 0xA5A5A5A5u;
    mock_reg_write(&ctx, LEDR_BASE, PIO_DATA_OFFSET, expected);
    uint32_t actual = fpga_peripheral_read(&ctx.bridge, LEDR_BASE, PIO_DATA_OFFSET);
    TEST_ASSERT_EQ_UINT(expected, actual, "read returns register contents");

    const uint32_t write_value = 0x55AA55AAu;
    fpga_peripheral_write(&ctx.bridge, LEDR_BASE, PIO_DATA_OFFSET, write_value);
    uint32_t written = mock_reg_read(&ctx, LEDR_BASE, PIO_DATA_OFFSET);
    TEST_ASSERT_EQ_UINT(write_value, written, "write updates register");

    mock_bridge_destroy(&ctx);
    return true;
}

static bool test_fpga_peripheral_read_write_guard(void) {
    mock_bridge_ctx_t ctx;
    TEST_ASSERT(mock_bridge_init(&ctx), "mock bridge should initialize");

    ctx.bridge.initialized = false;
    mock_reg_write(&ctx, LEDR_BASE, PIO_DATA_OFFSET, 0xCAFEBABEu);
    fpga_peripheral_write(&ctx.bridge, LEDR_BASE, PIO_DATA_OFFSET, 0xDEADBEEFu);
    uint32_t still_value = mock_reg_read(&ctx, LEDR_BASE, PIO_DATA_OFFSET);
    TEST_ASSERT_EQ_UINT(0xCAFEBABEu, still_value, "write ignored when bridge not initialized");

    uint32_t read_value = fpga_peripheral_read(&ctx.bridge, LEDR_BASE, PIO_DATA_OFFSET);
    TEST_ASSERT_EQ_UINT(0u, read_value, "read returns zero when bridge not initialized");

    mock_bridge_destroy(&ctx);
    return true;
}

static bool test_fpga_peripheral_null_safety(void) {
    fpga_peripheral_write(NULL, LEDR_BASE, PIO_DATA_OFFSET, 0x12345678u);
    uint32_t value = fpga_peripheral_read(NULL, LEDR_BASE, PIO_DATA_OFFSET);
    TEST_ASSERT_EQ_UINT(0u, value, "read with null bridge returns zero");
    volatile uint32_t *ptr = fpga_peripheral_get_register(NULL, LEDR_BASE, PIO_DATA_OFFSET);
    TEST_ASSERT(ptr == NULL, "get_register returns null for null bridge");
    return true;
}

static bool test_fpga_peripheral_get_register(void) {
    mock_bridge_ctx_t ctx;
    TEST_ASSERT(mock_bridge_init(&ctx), "mock bridge should initialize");

    volatile uint32_t *expected = mock_reg_ptr(&ctx, KEY_BASE, PIO_EDGE_OFFSET);
    volatile uint32_t *actual = fpga_peripheral_get_register(&ctx.bridge, KEY_BASE, PIO_EDGE_OFFSET);
    TEST_ASSERT(actual == expected, "get_register returns correct pointer");

    mock_bridge_destroy(&ctx);
    return true;
}

static bool test_fpga_leds_init_success(void) {
    mock_bridge_ctx_t ctx;
    TEST_ASSERT(mock_bridge_init(&ctx), "mock bridge should initialize");

    int rc = fpga_leds_init(&ctx.bridge);
    TEST_ASSERT(rc == 0, "leds init should succeed");
    TEST_ASSERT_EQ_UINT(0x3FFu, mock_reg_read(&ctx, LEDR_BASE, PIO_DIR_OFFSET),
                        "direction register configured for output");
    TEST_ASSERT_EQ_UINT(0u, mock_reg_read(&ctx, LEDR_BASE, PIO_DATA_OFFSET),
                        "data register cleared");

    mock_bridge_destroy(&ctx);
    return true;
}

static bool test_fpga_leds_init_failure(void) {
    mock_bridge_ctx_t ctx;
    TEST_ASSERT(mock_bridge_init(&ctx), "mock bridge should initialize");
    ctx.bridge.initialized = false;

    int rc = fpga_leds_init(&ctx.bridge);
    TEST_ASSERT(rc == -1, "leds init fails when bridge not initialized");

    rc = fpga_leds_init(NULL);
    TEST_ASSERT(rc == -1, "leds init fails with null bridge");

    mock_bridge_destroy(&ctx);
    return true;
}

static bool test_fpga_leds_set_and_get(void) {
    mock_bridge_ctx_t ctx;
    TEST_ASSERT(mock_bridge_init(&ctx), "mock bridge should initialize");

    fpga_leds_set(&ctx.bridge, 0xFFFFu);
    TEST_ASSERT_EQ_UINT(LED_ALL_MASK, mock_reg_read(&ctx, LEDR_BASE, PIO_DATA_OFFSET),
                        "leds set masks to LED_ALL_MASK");

    mock_reg_write(&ctx, LEDR_BASE, PIO_DATA_OFFSET, 0x155u);
    uint32_t current = fpga_leds_get(&ctx.bridge);
    TEST_ASSERT_EQ_UINT(0x155u, current, "leds get returns current value with mask");

    fpga_leds_set(NULL, 0x1u);
    uint32_t zero = fpga_leds_get(NULL);
    TEST_ASSERT_EQ_UINT(0u, zero, "leds get returns zero when bridge invalid");

    mock_bridge_destroy(&ctx);
    return true;
}

static bool test_fpga_keys_init_success(void) {
    mock_bridge_ctx_t ctx;
    TEST_ASSERT(mock_bridge_init(&ctx), "mock bridge should initialize");

    int rc = fpga_keys_init(&ctx.bridge);
    TEST_ASSERT(rc == 0, "keys init should succeed");
    TEST_ASSERT_EQ_UINT(0u, mock_reg_read(&ctx, KEY_BASE, PIO_DIR_OFFSET),
                        "keys configured as inputs");
    TEST_ASSERT_EQ_UINT(KEYS_MASK, mock_reg_read(&ctx, KEY_BASE, PIO_EDGE_OFFSET),
                        "edge capture cleared");

    mock_bridge_destroy(&ctx);
    return true;
}

static bool test_fpga_keys_read_behaviour(void) {
    mock_bridge_ctx_t ctx;
    TEST_ASSERT(mock_bridge_init(&ctx), "mock bridge should initialize");

    mock_reg_write(&ctx, KEY_BASE, PIO_DATA_OFFSET, KEYS_MASK);
    uint32_t none_pressed = fpga_keys_read(&ctx.bridge);
    TEST_ASSERT_EQ_UINT(KEYS_MASK, none_pressed, "keys read returns all ones when not pressed");

    mock_reg_write(&ctx, KEY_BASE, PIO_DATA_OFFSET, KEYS_MASK & ~KEY1_BIT);
    uint32_t one_pressed = fpga_keys_read(&ctx.bridge);
    TEST_ASSERT_EQ_UINT(KEYS_MASK & ~KEY1_BIT, one_pressed, "keys read returns masked value");

    uint32_t default_value = fpga_keys_read(NULL);
    TEST_ASSERT_EQ_UINT(KEYS_MASK, default_value, "keys read returns mask when bridge invalid");

    mock_bridge_destroy(&ctx);
    return true;
}

static bool test_fpga_keys_any_pressed(void) {
    mock_bridge_ctx_t ctx;
    TEST_ASSERT(mock_bridge_init(&ctx), "mock bridge should initialize");

    mock_reg_write(&ctx, KEY_BASE, PIO_DATA_OFFSET, KEYS_MASK);
    TEST_ASSERT(!fpga_keys_any_pressed(&ctx.bridge), "no keys pressed when mask set");

    mock_reg_write(&ctx, KEY_BASE, PIO_DATA_OFFSET, KEYS_MASK & ~KEY0_BIT);
    TEST_ASSERT(fpga_keys_any_pressed(&ctx.bridge), "any_pressed detects active-low press");

    TEST_ASSERT(!fpga_keys_any_pressed(NULL), "any_pressed returns false with invalid bridge");

    mock_bridge_destroy(&ctx);
    return true;
}

static bool test_fpga_keys_clear_edges(void) {
    mock_bridge_ctx_t ctx;
    TEST_ASSERT(mock_bridge_init(&ctx), "mock bridge should initialize");

    mock_reg_write(&ctx, KEY_BASE, PIO_EDGE_OFFSET, 0u);
    fpga_keys_clear_edges(&ctx.bridge);
    TEST_ASSERT_EQ_UINT(KEYS_MASK, mock_reg_read(&ctx, KEY_BASE, PIO_EDGE_OFFSET),
                        "clear edges writes mask");

    fpga_keys_clear_edges(NULL);
    mock_bridge_destroy(&ctx);
    return true;
}

static bool test_fpga_switches_init_success(void) {
    mock_bridge_ctx_t ctx;
    TEST_ASSERT(mock_bridge_init(&ctx), "mock bridge should initialize");

    int rc = fpga_switches_init(&ctx.bridge);
    TEST_ASSERT(rc == 0, "switches init should succeed");
    TEST_ASSERT_EQ_UINT(0u, mock_reg_read(&ctx, SW_BASE, PIO_DIR_OFFSET),
                        "switch direction configured as inputs");

    mock_bridge_destroy(&ctx);
    return true;
}

static bool test_fpga_switches_read(void) {
    mock_bridge_ctx_t ctx;
    TEST_ASSERT(mock_bridge_init(&ctx), "mock bridge should initialize");

    mock_reg_write(&ctx, SW_BASE, PIO_DATA_OFFSET, 0x3FFu);
    uint32_t value = fpga_switches_read(&ctx.bridge);
    TEST_ASSERT_EQ_UINT(SWITCHES_MASK, value, "switches read returns masked value");

    ctx.bridge.initialized = false;
    uint32_t default_value = fpga_switches_read(&ctx.bridge);
    TEST_ASSERT_EQ_UINT(SWITCHES_MASK, default_value,
                        "switches read returns mask when bridge invalid");

    mock_bridge_destroy(&ctx);
    return true;
}

typedef bool (*test_fn)(void);

typedef struct {
    const char *name;
    test_fn fn;
} test_case_t;

static const test_case_t TESTS[] = {
    {"fpga_peripheral_read_write_basic", test_fpga_peripheral_read_write_basic},
    {"fpga_peripheral_read_write_guard", test_fpga_peripheral_read_write_guard},
    {"fpga_peripheral_null_safety", test_fpga_peripheral_null_safety},
    {"fpga_peripheral_get_register", test_fpga_peripheral_get_register},
    {"fpga_leds_init_success", test_fpga_leds_init_success},
    {"fpga_leds_init_failure", test_fpga_leds_init_failure},
    {"fpga_leds_set_and_get", test_fpga_leds_set_and_get},
    {"fpga_keys_init_success", test_fpga_keys_init_success},
    {"fpga_keys_read_behaviour", test_fpga_keys_read_behaviour},
    {"fpga_keys_any_pressed", test_fpga_keys_any_pressed},
    {"fpga_keys_clear_edges", test_fpga_keys_clear_edges},
    {"fpga_switches_init_success", test_fpga_switches_init_success},
    {"fpga_switches_read", test_fpga_switches_read},
};

int main(void) {
    size_t total = sizeof(TESTS) / sizeof(TESTS[0]);
    size_t passed = 0;

    for (size_t i = 0; i < total; ++i) {
        bool ok = TESTS[i].fn();
        printf("[%s] %s\n", ok ? "PASS" : "FAIL", TESTS[i].name);
        if (ok) {
            passed++;
        }
    }

    printf("\nHardware tests: %zu/%zu passed\n", passed, total);
    return (passed == total) ? 0 : 1;
}


