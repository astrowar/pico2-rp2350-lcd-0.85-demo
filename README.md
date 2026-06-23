# Raspberry Pi Pico 2 (RP2350) 0.85-inch LCD Demo

This is a standalone, clean C++ demonstration project for the [Raspberry Pi Pico 2 (RP2350A) with a integrated 0.85-inch LCD Display Screen](https://spotpear.com/shop/Raspberry-Pi-Pico-2-RP2350A-LCD-0.85-inch-Display-Screen.html).

The board features a high-density 128x128 pixel, 0.85" color LCD screen driven by the **GC9107** display driver over SPI.

This example utilizes the lightweight, highly optimized [displaylib_16bit_PICO](https://github.com/gavinlyonsrepo/displaylib_16bit_PICO) graphics library to render vibrant color tests, outlines, text elements, crosses, and shapes without bulky, copy-pasted driver code.

---

## Hardware Pinout

The integrated display on the RP2350 board is pre-routed internally to the following micro-controller pins:

| Display Pin | Pico 2 / RP2350A Pin | Function / Description |
| :--- | :--- | :--- |
| **SCLK** | GPIO 14 | SPI1 Serial Clock |
| **MOSI (DIN)** | GPIO 15 | SPI1 Master Out Slave In |
| **CS** | GPIO 13 | Chip Select (Active Low) |
| **DC** | GPIO 12 | Data / Command Select |
| **RST** | GPIO 10 | Hardware Reset (Active Low) |
| **MISO** | *N/A (unconnected)*| Not used by this write-only display |

---

## Getting Started

### 1. Prerequisites
Ensure you have the [Raspberry Pi Pico SDK v2.0.0+](https://github.com/raspberrypi/pico-sdk) or later installed and configured on your machine (specifically, setting the `PICO_SDK_PATH` environment variable).

### 2. Cloning the Repository
Because this project utilizes an external library as a Git submodule, you **must** clone the repository with the `--recursive` flag to pull the dependency:

```bash
# Clone the repository including the submodule
git clone --recursive <repository-url>

# Alternatively, if you already cloned without submodules, run:
git submodule update --init --recursive
```

---

## Building and Compiling

The project compiles with CMake. Follow the standard Pico compilation workflow:

```bash
# 1. Create a build directory
mkdir build
cd build

# 2. Configure the project for the RP2350 (Pico 2)
cmake -DPICO_BOARD=pico2 ..

# 3. Compile the executable
make -j$(nproc)
```

This will produce the standard Pico build files inside the `build/` folder:
- `pico_screen_test.elf` (for debugging)
- `pico_screen_test.uf2` (for drag-and-drop programming)

---

## Flashing

1. Connect your Raspberry Pi Pico 2 / RP2350A to your computer while holding down the **BOOTSEL** button.
2. The board will mount as a mass storage device.
3. Drag and drop the compiled `pico_screen_test.uf2` file onto the mounted drive.
4. The board will automatically reboot and start running the continuous color and graphics test loops.

---

## Code Structure

- `pico_screen_test.cpp`: Core application main loop containing the hardware definitions, display setup, and screen test patterns (rainbow stripes, gradients, grid frames, diagonal crosses, circles, and text elements).
- `CMakeLists.txt`: Project build recipe configuring libraries, hardware SPI peripherals, and referencing the git submodule library.
- `lib/displaylib_16bit_PICO`: Git submodule of the display driver library.
- `.vscode/`: Standard IDE build and debug profiles.
