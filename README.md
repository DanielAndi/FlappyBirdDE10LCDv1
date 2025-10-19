# Flappy Game on DE10-Standard (Cyclone V SoC)
**Embedded Systems Final Project — Implemented Fully in C**

---

## Overview
This project implements a simplified *Flappy Bird*-style game on the **DE10-Standard development board**, built entirely in **C** and designed to run directly on the **HPS (ARM Cortex-A9)** processor.

All hardware communication is done through **memory-mapped I/O** using the **Cyclone V SoC address map**, with no high-level libraries or operating system dependencies.  
Originally designed for VGA output, this version has been adapted to use the **on-board LCD display**, different from the initial idea of using the VGA output making the project fully self-contained.

---

## Project Objectives
- Develop a functional embedded game using C and direct hardware access.
- Interface the **HPS and FPGA** through the **lightweight bridge**.
- Display live gameplay on the **DE10-Standard LCD** via the HPS SPI controller.
- Handle **inputs** (buttons, switches) and **outputs** (LEDs, LCD) through the system address map.
- Demonstrate end-to-end software/hardware integration within the Cyclone V SoC.

---

## System Components

| Component | Function | Interface / Address Range |
|------------|-----------|---------------------------|
| **HPS ARM Cortex-A9** | Runs main game logic | Linux (via SSH) |
| **HPS–FPGA Bridge (LW)** | Communication with FPGA peripherals | 0xFF200000 – 0xFF20FFFF |
| **LEDs (LEDR[9:0])** | Visual status / debug output | 0xFF200000 |
| **Switches (SW[9:0])** | Control difficulty / parameters | 0xFF200040 |
| **Pushbuttons (KEY[3:0])** | Player input (flap, reset) | 0xFF200050 |
| **LCD Display (128×64)** | Game display | HPS SPI0 |
| **Timer (HPS Timer0)** | Frame timing / delays | 0xFFC08000 |

---

## Gameplay Description
The player controls a small pixel character (the “bird”) on the LCD.  
Using **pushbuttons**, the player flaps to avoid incoming obstacles represented as gaps in vertical columns.  
**Slide switches** control difficulty or speed, while **LEDs** reflect score or life indicators.

---

## Software Architecture

### 1. Initialization
- Enable **HPS↔FPGA bridges** by clearing bits [1:0] at `0xFFD0501C`.
- Map **lightweight bridge** base `0xFF200000` via `/dev/mem` and `mmap()`.
- Initialize **SPI0** to communicate with the LCD display.
- Clear screen and prepare display buffer.

### 2. Main Loop
- Poll **KEY** and **SW** inputs.
- Update player position and obstacles.
- Render frame buffer (128×8 pages) in DDR memory.
- Transmit frame data to LCD over SPI0.
- Update **LEDs** with score or status.

### 3. Timing
- Frame updates paced using **HPS Timer0** or software delay.
- Approximate frame rate: 20–30 FPS.

---

## Development Setup

| Tool | Description |
|------|--------------|
| **Board:** | Terasic DE10-Standard (Cyclone V SoC) |
| **Language:** | C |
| **Compiler:** | GCC for ARM (on-board or cross-compiled) |
| **Connection:** | SSH from host computer |
| **Build Command:** | `gcc main.c -O2 -o flappy` |
| **Run Command:** | `sudo ./flappy` |

## Use keys/switches
- KEY0: Jump / Flap
- KEY1: Reset
- SW[0–3]: Speed / difficulty
- LEDs: Score / lives