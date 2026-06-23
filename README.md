# Raspberry Pi Pico 2 (RP2350) 0.85-inch LCD Demos

This repository is a clean C++ workspace containing two distinct demonstration programs for the [Raspberry Pi Pico 2 (RP2350A) with an integrated 0.85-inch LCD Display Screen](https://spotpear.com/shop/Raspberry-Pi-Pico-2-RP2350A-LCD-0.85-inch-Display-Screen.html).

The board features an on-board high-density 128x128 pixel color LCD screen driven by the **GC9107** display driver, a **QMI8658A** 6-axis IMU (gyroscope + accelerometer), and an RGB WS2812 status LED.

These examples utilize the lightweight, highly optimized [displaylib_16bit_PICO](https://github.com/gavinlyonsrepo/displaylib_16bit_PICO) graphics library to render smooth real-time visual outputs without copy-pasted low-level driver bloat.

---

## 🚀 Available Demos / Targets

This workspace is structured as a multi-target CMake project containing two separate executables:

### 1. `pico_screen_test` (Display Diagnostics)
A continuous graphics loop showing full color-fili, diagnostic borders, crosshairs, concentric circles, custom text, and a rendering performance test checkerboard.
* **Key File:** `pico_screen_test.cpp`

### 2. `pico_giroscope` (Real-time Level Meter)
A 3D complementary-filter level meter. It reads roll and pitch angles from the on-board **QMI8658A** 6-axis IMU over I2C at 50Hz, calculates raw degrees, filters noise, and renders a real-time bubble level (container circle, crosshairs, and a moving bubble).
* **Key File:** `pico_giroscope.cpp`

---

## Hardware Pinout

The integrated components on this specialized RP2350A board are pre-routed internally to the following micro-controller pins:

### GC9107 LCD Display (SPI1)
| Display Pin | Pico 2 / RP2350 Pin | Function / Description |
| :--- | :--- | :--- |
| **SCLK** | GPIO 14 | SPI1 Serial Clock |
| **MOSI (DIN)** | GPIO 15 | SPI1 Master Out Slave In |
| **CS** | GPIO 13 | Chip Select (Active Low) |
| **DC** | GPIO 12 | Data / Command Select |
| **RST** | GPIO 10 | Hardware Reset (Active Low) |
| **MISO** | *N/A (unconnected)*| Not used by this write-only display |

### QMI8658A 6-axis IMU (I2C0)
| Sensor Pin | Pico 2 / RP2350 Pin | Function / Description |
| :--- | :--- | :--- |
| **SDA** | GPIO 20 | I2C0 Serial Data |
| **SCL** | GPIO 21 | I2C0 Serial Clock |

### WS2812 Status LED
| Pin | Pico 2 / RP2350 Pin | Function / Description |
| :--- | :--- | :--- |
| **DIN** | GPIO 25 | SIO Bitbang Status Signal |

---

## Getting Started

### 1. Prerequisites
Ensure you have the [Raspberry Pi Pico SDK v2.0.0+](https://github.com/raspberrypi/pico-sdk) or later installed and configured on your machine (specifically, setting the `PICO_SDK_PATH` environment variable).

### 2. Cloning the Repository
Because this project utilizes the external graphics library as a Git submodule, you **must** clone the repository recursively:

```bash
# Clone the repository including the submodule
git clone --recursive <repository-url>

# Alternatively, if you already cloned without submodules, run:
git submodule update --init --recursive
```

---

## Building and Compiling

The project compiles with CMake. You can build both demos simultaneously or choose a specific target:

```bash
# 1. Create a build directory
mkdir build
cd build

# 2. Configure the project for the RP2350 (Pico 2)
cmake -DPICO_BOARD=pico2 ..

# 3. Compile all targets
make -j$(nproc)
```

### To build only a specific target:
```bash
# To build ONLY the screen test:
cmake --build . --target pico_screen_test

# To build ONLY the gyroscope level meter:
cmake --build . --target pico_giroscope
```

This will produce the compiled files inside your `build/` directory:
* For Screen Test: `pico_screen_test.elf`, `pico_screen_test.uf2`
* For Gyroscope: `pico_giroscope.elf`, `pico_giroscope.uf2`

---

## Flashing

1. Connect your Raspberry Pi Pico 2 / RP2350A board to your computer while holding down the **BOOTSEL** button.
2. The board will mount as a mass storage device (RPI-RP2).
3. Drag and drop either `pico_screen_test.uf2` or `pico_giroscope.uf2` onto the mounted drive.
4. The board will automatically reboot and start running your selected demo!
