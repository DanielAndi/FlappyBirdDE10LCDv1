# Flappy Bird on DE10-Standard

**Embedded Systems Final Project — Implemented Fully in C**

A Flappy Bird-style game implemented for the **DE10-Standard development board** (Cyclone V SoC), running entirely in **C** on the **HPS (ARM Cortex-A9)** processor with direct hardware access through memory-mapped I/O.

---

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Hardware Requirements](#hardware-requirements)
- [System Architecture](#system-architecture)
- [Project Structure](#project-structure)
- [Build Instructions](#build-instructions)
- [Usage](#usage)
- [Controls](#controls)
- [Software Architecture](#software-architecture)
- [Development](#development)
- [License](#license)

---

## Overview

This project implements a simplified Flappy Bird game that runs directly on the DE10-Standard FPGA development board. The game uses:

- **HPS (Hard Processor System)**: ARM Cortex-A9 processor running embedded Linux
- **FPGA Peripherals**: LEDs, pushbuttons, switches accessed via the lightweight bridge
- **LCD Display**: 128×64 pixel on-board LCD display for game rendering
- **Direct Hardware Access**: Memory-mapped I/O with no high-level libraries

All hardware communication is performed through **memory-mapped I/O** using the **Cyclone V SoC address map**, providing direct control over FPGA peripherals and the LCD display.

---

## Features

- ✅ Real-time game loop running at 60 FPS
- ✅ Physics-based bird movement with gravity
- ✅ Collision detection using AABB (Axis-Aligned Bounding Box)
- ✅ Random pipe generation with gaps
- ✅ Score tracking and display
- ✅ Startup screen and game over handling
- ✅ Graceful signal handling (SIGINT/SIGTERM)
- ✅ Clean modular architecture

---

## Hardware Requirements

- **Board**: Terasic DE10-Standard (Cyclone V SoC)
- **Processor**: ARM Cortex-A9 (HPS)
- **Operating System**: Embedded Linux (on-board)
- **Connection**: SSH access from host computer
- **Permissions**: Root privileges required for `/dev/mem` access

---

## System Architecture

### Hardware Components

| Component | Function | Address Range |
|-----------|----------|---------------|
| **HPS ARM Cortex-A9** | Runs game logic and Linux OS | - |
| **HPS–FPGA Bridge (LW)** | Communication with FPGA peripherals | 0xFF200000 – 0xFF20FFFF |
| **LEDs (LEDR[9:0])** | Visual status / score display | 0xFF200000 |
| **Switches (SW[9:0])** | Game configuration / difficulty | 0xFF200040 |
| **Pushbuttons (KEY[3:0])** | Player input (flap/jump) | 0xFF200050 |
| **LCD Display (128×64)** | Game rendering | HPS SPI0 |
| **7-Segment Displays** | Score display (future use) | 0xFF200020, 0xFF200030 |

### Communication Flow

```
HPS Application
    ↓
FPGA Bridge (mmap /dev/mem)
    ↓
Lightweight Bridge (0xFF200000)
    ↓
FPGA Peripherals (LEDs, Keys, Switches)
```

---

## Project Structure

```
FlappyBirdDE10LCDv1/
├── main.c                 # Main entry point and initialization
├── Makefile              # Build configuration
├── README.md             # This file
│
├── assets/               # Game assets
│   └── sprites_data.h    # Sprite bitmap data
│
├── display/              # Display subsystem
│   ├── graphics.h/c      # High-level graphics API
│   ├── lcd_driver.h/c    # Low-level LCD driver
│   └── sprites.h/c       # Sprite management
│
├── game/                 # Game logic
│   ├── gameplay.h/c      # Main game loop
│   ├── game_objects.h/c  # Bird and pipe entities
│   ├── physics.h/c       # Physics engine
│   ├── collision.h/c     # Collision detection
│   └── game_state.h/c    # Game state management
│
├── hardware/             # Hardware abstraction
│   ├── hps_fpga_bridge.h/c    # FPGA bridge interface
│   ├── fpga_peripherals.h/c   # FPGA peripheral access
│   ├── address_map_arm.h      # Memory map definitions
│   └── tests/                 # Hardware test utilities
│
└── input/                # Input handling
    └── input_handler.h/c # Button and switch input
```

---

## Build Instructions

### Prerequisites

- SSH access to DE10-Standard board
- GCC compiler (ARM or cross-compiler)
- Make utility
- Root/sudo access on the board

### Building

1. **Clone or copy the project to the board:**
   ```bash
   # On the board or via SSH
   cd /path/to/FlappyBirdDE10LCDv1
   ```

2. **Build the project:**
   ```bash
   make
   ```

   This will compile all source files and create the `flappy_bird` executable.

3. **Build hardware tests (optional):**
   ```bash
   make hardware-tests
   ```

### Build Options

The Makefile supports the following targets:

- `make` or `make flappy_bird`: Build the main game executable
- `make hardware-tests`: Build hardware test utilities
- `make test-hardware`: Build and run hardware tests
- `make clean`: Remove build artifacts

---

## Usage

### Running the Game

1. **Ensure you have root privileges:**
   ```bash
   sudo ./flappy_bird
   ```

   **Note:** Root privileges are required to access `/dev/mem` for FPGA bridge communication.

2. **The game will:**
   - Initialize hardware peripherals
   - Display a startup screen
   - Wait for button press to start
   - Run the game loop
   - Handle cleanup on exit

3. **To exit gracefully:**
   - Press `Ctrl+C` (SIGINT)
   - Or send SIGTERM signal

### Running Hardware Tests

```bash
sudo make test-hardware
```

This will run hardware tests to verify FPGA peripheral access.

---

## Controls

| Input | Function |
|-------|----------|
| **KEY0** | Jump / Flap (primary game control) |
| **KEY1** | Reset (future use) |
| **KEY2-KEY3** | Reserved for future use |
| **SW[0-9]** | Game configuration / difficulty (future use) |
| **LEDs** | Score display / status indicators |

---

## Software Architecture

### Module Overview

#### 1. **Main (`main.c`)**
   - Program entry point
   - Signal handling setup
   - Subsystem initialization
   - Main execution flow

#### 2. **Hardware Layer (`hardware/`)**
   - **`hps_fpga_bridge`**: Low-level FPGA bridge access via mmap
   - **`fpga_peripherals`**: High-level peripheral access (LEDs, keys, switches)
   - **`address_map_arm`**: Memory map address definitions

#### 3. **Display Layer (`display/`)**
   - **`lcd_driver`**: Low-level LCD driver (SPI communication)
   - **`graphics`**: High-level graphics API (pixels, sprites, text)
   - **`sprites`**: Sprite data management

#### 4. **Game Logic (`game/`)**
   - **`gameplay`**: Main game loop and state machine
   - **`game_objects`**: Bird and pipe entity management
   - **`physics`**: Simple physics engine (Euler integration)
   - **`collision`**: AABB collision detection
   - **`game_state`**: Game state management

#### 5. **Input Layer (`input/`)**
   - **`input_handler`**: Button and switch polling and state management

### Game Loop

The main game loop (60 FPS) performs:

1. **Input Polling**: Read button and switch states
2. **Physics Update**: Update bird position and velocity
3. **Pipe Update**: Move pipes and handle respawning
4. **Collision Detection**: Check bird-pipe collisions
5. **Scoring**: Update score when bird passes pipes
6. **Rendering**: Draw all game objects to frame buffer
7. **Display Update**: Flush frame buffer to LCD
8. **Timing**: Wait for next frame

### Initialization Sequence

1. Install signal handlers (SIGINT, SIGTERM)
2. Initialize FPGA bridge (`fpga_bridge_init`)
3. Initialize LEDs (`fpga_leds_init`)
4. Initialize graphics subsystem (`graphics_init`)
5. Display startup screen (`graphics_draw_startup_screen`)
6. Initialize input handler (`input_handler_init`)
7. Wait for user input to start
8. Run game loop (`gameplay_run`)
9. Cleanup and exit

---

## Development

### Code Style

- **Language**: C99 standard
- **Indentation**: 4 spaces
- **Naming**: `snake_case` for functions and variables
- **Documentation**: Doxygen-style comments
- **Error Handling**: Return codes (0 = success, -1 = error)

### Adding Features

#### Adding a New Sprite

1. Add sprite data to `assets/sprites_data.h`
2. Define sprite dimensions (width, height)
3. Add accessor function in `display/sprites.c`
4. Use `graphics_draw_sprite()` to render

#### Adding a New Game Object

1. Define structure in `game/game_objects.h`
2. Implement init/update functions in `game/game_objects.c`
3. Add to game loop in `game/gameplay.c`

#### Modifying Physics

1. Adjust constants in `game/gameplay.c`:
   - `GRAVITY_ACCELERATION`: Gravity strength
   - `JUMP_IMPULSE`: Jump/flap strength
   - `PIPE_SPEED`: Pipe movement speed

### Debugging

- Use LEDs for status indication
- Add debug output via `fprintf(stderr, ...)`
- Use hardware tests to verify peripheral access
- Check signal handling with `strace`

### Testing

```bash
# Build and run hardware tests
sudo make test-hardware

# Run game with debug output
sudo ./flappy_bird 2>&1 | tee game.log
```

---

## Technical Details

### Frame Rate

- **Target**: 60 FPS
- **Frame Time**: ~16.67 ms per frame
- **Timing Method**: `usleep()` with frame time calculation

### Physics

- **Integration**: Euler method
- **Gravity**: 2000 pixels/s² (downward)
- **Jump Impulse**: -250 pixels/s (upward)
- **Pipe Speed**: 55 pixels/s (leftward)

### Collision Detection

- **Method**: Axis-Aligned Bounding Box (AABB)
- **Objects**: Bird bounds, pipe bounds (top and bottom)
- **Test**: Rectangle overlap detection

### Display

- **Resolution**: 128×64 pixels
- **Interface**: SPI (HPS SPI0)
- **Buffer**: Frame buffer in system memory
- **Update**: Full frame flush to LCD

---

## License

This project is an educational implementation for embedded systems coursework.

---

## Acknowledgments

- **Board**: Terasic DE10-Standard
- **Processor**: Intel/Altera Cyclone V SoC
- **Game Concept**: Inspired by Flappy Bird by Dong Nguyen

---

## Troubleshooting

### Common Issues

**Issue**: "Failed to initialize FPGA bridge"
- **Solution**: Ensure you're running with root privileges (`sudo`)

**Issue**: "Failed to initialize LEDs"
- **Solution**: Verify FPGA bridge initialization succeeded

**Issue**: Game runs slowly or stutters
- **Solution**: Check system load, ensure no other processes are consuming CPU

**Issue**: Buttons don't respond
- **Solution**: Verify input handler initialization, check button hardware

**Issue**: Display shows garbage or nothing
- **Solution**: Check LCD connection, verify SPI communication, check frame buffer

---

## Future Enhancements

- [ ] High score persistence
- [ ] Multiple difficulty levels
- [ ] Sound effects
- [ ] Background scrolling
- [ ] Power-ups and bonuses
- [ ] Multiplayer support
- [ ] Configurable controls
- [ ] Game state save/load

---

## Contact

For questions or issues, please refer to the project documentation or contact the development team.

---

**Last Updated**: 2025
**Version**: 1.0
