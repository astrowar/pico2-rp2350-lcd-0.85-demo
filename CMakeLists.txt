# Generated CMake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2 CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(pico_screen_test C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable
add_executable(pico_screen_test pico_screen_test.cpp)

pico_set_program_name(pico_screen_test "pico_screen_test")
pico_set_program_version(pico_screen_test "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(pico_screen_test 1)
pico_enable_stdio_usb(pico_screen_test 1)

# Define display library target from submodule files
add_library(pico_displaylib_16 INTERFACE)

target_sources(pico_displaylib_16 INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/lib/displaylib_16bit_PICO/src/displaylib_16/st7735.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/displaylib_16bit_PICO/src/displaylib_16/st7789.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/displaylib_16bit_PICO/src/displaylib_16/ili9341.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/displaylib_16bit_PICO/src/displaylib_16/ssd1331.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/displaylib_16bit_PICO/src/displaylib_16/gc9a01.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/displaylib_16bit_PICO/src/displaylib_16/gc9d01.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/displaylib_16bit_PICO/src/displaylib_16/gc9107.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/displaylib_16bit_PICO/src/displaylib_16/displaylib_16_graphics.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/displaylib_16bit_PICO/src/displaylib_16/displaylib_16_Print.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/displaylib_16bit_PICO/src/displaylib_16/displaylib_16_Font.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/displaylib_16bit_PICO/src/displaylib_16/displaylib_16_common.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/displaylib_16bit_PICO/src/displaylib_16/Bitmap_TEST_Data.cpp
)

target_include_directories(pico_displaylib_16 INTERFACE 
  ${CMAKE_CURRENT_LIST_DIR}/lib/displaylib_16bit_PICO/include
)

# Add inclusion of the current list directory (project root)
target_include_directories(pico_screen_test PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
)

# Link everything to executable
target_link_libraries(pico_screen_test
  pico_stdlib
  hardware_spi
  hardware_gpio
  pico_displaylib_16
)

pico_add_extra_outputs(pico_screen_test)
