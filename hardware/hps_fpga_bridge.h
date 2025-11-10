/**
 * @file hps_fpga_bridge.h
 * @brief HPS-to-FPGA Communication Bridge Abstraction Layer
 * 
 * This module provides an abstraction layer for communication between the
 * Hard Processor System (HPS) and the FPGA via the Lightweight Bridge (LW_BRIDGE).
 * 
 * Architecture Overview:
 * ----------------------
 * The Cyclone V SoC provides two communication bridges between HPS and FPGA:
 * 1. Lightweight Bridge (LW_BRIDGE) - Base: 0xFF200000
 *    - Used for low-bandwidth, memory-mapped I/O
 *    - Accesses FPGA peripherals (LEDs, Keys, Switches, etc.)
 *    - HPS reads/writes to LW_BRIDGE addresses which are forwarded to FPGA
 * 
 * 2. Heavyweight Bridge (HW_BRIDGE) - Not used in this project
 *    - Used for high-bandwidth data transfers
 * 
 * Communication Flow:
 * -------------------
 * HPS Application -> mmap(/dev/mem) -> LW_BRIDGE (0xFF200000) -> FPGA Peripherals
 * 
 * Memory Mapping Process:
 * 1. Open /dev/mem to access physical memory
 * 2. Map LW_BRIDGE region (0xFF200000) into virtual address space
 * 3. Access FPGA peripherals by adding their offsets to the mapped base address
 * 
 * FPGA Peripheral Access:
 * -----------------------
 * Each FPGA peripheral (LEDR, KEY, etc.) has:
 * - A base offset from LW_BRIDGE_BASE
 * - Register offsets for Data (0x0), Direction (0x4), Edge (0xC), etc.
 * 
 * Example: Accessing LEDR
 *   virtual_base = mmap(LW_BRIDGE_BASE)
 *   ledr_data = virtual_base + LEDR_BASE + PIO_DATA_OFFSET
 *   *ledr_data = 0x3FF;  // Turn on all LEDs
 */

#ifndef HPS_FPGA_BRIDGE_H
#define HPS_FPGA_BRIDGE_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Opaque handle for the FPGA bridge connection
 * 
 * This structure encapsulates the memory-mapped bridge connection.
 * Applications should treat this as an opaque type and use the provided
 * functions to interact with it.
 */
typedef struct {
    void *virtual_base;    ///< Virtual address of mapped LW_BRIDGE region
    int mem_fd;            ///< File descriptor for /dev/mem
    bool initialized;      ///< Initialization status flag
} fpga_bridge_t;

/**
 * @brief Initialize the HPS-to-FPGA bridge connection
 * 
 * This function:
 * 1. Opens /dev/mem to access physical memory
 * 2. Maps the LW_BRIDGE region (0xFF200000) into virtual address space
 * 3. Returns a handle for subsequent FPGA peripheral access
 * 
 * @param bridge Pointer to bridge structure to initialize
 * @return 0 on success, -1 on failure
 * 
 * @note This function must be called before any FPGA peripheral access
 * @note Requires root privileges to open /dev/mem
 */
int fpga_bridge_init(fpga_bridge_t *bridge);

/**
 * @brief Clean up and close the FPGA bridge connection
 * 
 * This function:
 * 1. Unmaps the LW_BRIDGE memory region
 * 2. Closes the /dev/mem file descriptor
 * 3. Resets the bridge structure
 * 
 * @param bridge Pointer to bridge structure to clean up
 * @return 0 on success, -1 on failure
 */
int fpga_bridge_cleanup(fpga_bridge_t *bridge);

/**
 * @brief Read from an FPGA peripheral register
 * 
 * Reads a 32-bit value from an FPGA peripheral register.
 * 
 * @param bridge Initialized bridge handle
 * @param peripheral_base Base offset of the peripheral (e.g., LEDR_BASE)
 * @param register_offset Register offset within the peripheral (e.g., PIO_DATA_OFFSET)
 * @return 32-bit register value
 * 
 * @note The address calculation is:
 *   address = bridge->virtual_base + peripheral_base + register_offset
 */
uint32_t fpga_peripheral_read(fpga_bridge_t *bridge, uint32_t peripheral_base, uint32_t register_offset);

/**
 * @brief Write to an FPGA peripheral register
 * 
 * Writes a 32-bit value to an FPGA peripheral register.
 * 
 * @param bridge Initialized bridge handle
 * @param peripheral_base Base offset of the peripheral (e.g., LEDR_BASE)
 * @param register_offset Register offset within the peripheral (e.g., PIO_DATA_OFFSET)
 * @param value 32-bit value to write
 * 
 * @note The address calculation is:
 *   address = bridge->virtual_base + peripheral_base + register_offset
 */
void fpga_peripheral_write(fpga_bridge_t *bridge, uint32_t peripheral_base, uint32_t register_offset, uint32_t value);

/**
 * @brief Get a pointer to an FPGA peripheral register
 * 
 * Returns a volatile pointer to an FPGA peripheral register for direct access.
 * This is useful when you need to perform multiple operations on the same register.
 * 
 * @param bridge Initialized bridge handle
 * @param peripheral_base Base offset of the peripheral (e.g., LEDR_BASE)
 * @param register_offset Register offset within the peripheral (e.g., PIO_DATA_OFFSET)
 * @return Volatile pointer to the register, or NULL if bridge is not initialized
 * 
 * @warning The returned pointer becomes invalid after fpga_bridge_cleanup()
 */
volatile uint32_t* fpga_peripheral_get_register(fpga_bridge_t *bridge, uint32_t peripheral_base, uint32_t register_offset);

#endif /* HPS_FPGA_BRIDGE_H */


