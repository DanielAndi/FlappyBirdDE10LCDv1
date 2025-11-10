/**
 * @file hps_fpga_bridge.c
 * @brief Implementation of HPS-to-FPGA communication bridge
 * 
 * This module implements the abstraction layer for HPS-FPGA communication
 * via the Lightweight Bridge (LW_BRIDGE). It provides functions to initialize
 * the bridge, map memory regions, and access FPGA peripherals.
 * 
 * @author Daniel Grijalva
 * @date 2025
 * @version 1.0
 */

#include "hps_fpga_bridge.h"
#include "address_map_arm.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>

int fpga_bridge_init(fpga_bridge_t *bridge) {
    if (bridge == NULL) {
        printf("FPGA Bridge: ERROR - null bridge pointer\n");
        return -1;
    }

    memset(bridge, 0, sizeof(fpga_bridge_t));

    bridge->mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (bridge->mem_fd == -1) {
        printf("FPGA Bridge: ERROR - could not open /dev/mem\n");
        printf("FPGA Bridge: Note - this requires root privileges\n");
        return -1;
    }

    bridge->virtual_base = mmap(NULL,
                                LW_BRIDGE_SPAN,
                                PROT_READ | PROT_WRITE,
                                MAP_SHARED,
                                bridge->mem_fd,
                                LW_BRIDGE_BASE);

    if (bridge->virtual_base == MAP_FAILED) {
        printf("FPGA Bridge: ERROR - mmap failed for LW_BRIDGE\n");
        printf("FPGA Bridge: Physical address: 0x%08X, Span: 0x%08X\n",
               LW_BRIDGE_BASE, LW_BRIDGE_SPAN);
        close(bridge->mem_fd);
        bridge->mem_fd = -1;
        return -1;
    }

    bridge->initialized = true;
    printf("FPGA Bridge: Initialized successfully\n");
    printf("FPGA Bridge: Mapped LW_BRIDGE at physical 0x%08X to virtual %p\n",
           LW_BRIDGE_BASE, bridge->virtual_base);

    return 0;
}

int fpga_bridge_cleanup(fpga_bridge_t *bridge) {
    if (bridge == NULL || !bridge->initialized) {
        return 0;
    }

    if (bridge->virtual_base != NULL) {
        if (munmap(bridge->virtual_base, LW_BRIDGE_SPAN) != 0) {
            printf("FPGA Bridge: WARNING - munmap failed\n");
            return -1;
        }
        printf("FPGA Bridge: Unmapped LW_BRIDGE\n");
        bridge->virtual_base = NULL;
    }

    if (bridge->mem_fd != -1) {
        if (close(bridge->mem_fd) != 0) {
            printf("FPGA Bridge: WARNING - failed to close /dev/mem\n");
            return -1;
        }
        printf("FPGA Bridge: Closed /dev/mem\n");
        bridge->mem_fd = -1;
    }

    bridge->initialized = false;
    printf("FPGA Bridge: Cleanup complete\n");

    return 0;
}

uint32_t fpga_peripheral_read(fpga_bridge_t *bridge,
                              uint32_t peripheral_base,
                              uint32_t register_offset) {
    if (bridge == NULL || !bridge->initialized || bridge->virtual_base == NULL) {
        printf("FPGA Bridge: ERROR - bridge not initialized\n");
        return 0;
    }

    volatile uint32_t *reg = (volatile uint32_t *)((uint8_t *)bridge->virtual_base +
                                                   peripheral_base +
                                                   register_offset);

    return *reg;
}

void fpga_peripheral_write(fpga_bridge_t *bridge,
                           uint32_t peripheral_base,
                           uint32_t register_offset,
                           uint32_t value) {
    if (bridge == NULL || !bridge->initialized || bridge->virtual_base == NULL) {
        printf("FPGA Bridge: ERROR - bridge not initialized\n");
        return;
    }

    volatile uint32_t *reg = (volatile uint32_t *)((uint8_t *)bridge->virtual_base +
                                                   peripheral_base +
                                                   register_offset);

    *reg = value;
}

volatile uint32_t* fpga_peripheral_get_register(fpga_bridge_t *bridge,
                                                uint32_t peripheral_base,
                                                uint32_t register_offset) {
    if (bridge == NULL || !bridge->initialized || bridge->virtual_base == NULL) {
        printf("FPGA Bridge: ERROR - bridge not initialized\n");
        return NULL;
    }

    return (volatile uint32_t *)((uint8_t *)bridge->virtual_base +
                                 peripheral_base +
                                 register_offset);
}


