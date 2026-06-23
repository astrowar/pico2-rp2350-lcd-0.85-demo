/*!
    @file    pico_screen_test.cpp
    @author  Adapted to use displaylib_16bit_PICO library
    @brief   Example using GC9107 display driver with pico_screen_test project
    @details Setup for 128x128 GC9107 display (0.85" display pico--260317)
    @note    GPIO pins match the manufacturer's display board
*/

#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "displaylib_16/gc9107.hpp"

// SPI and GPIO defines
#define SPI_PORT spi1
#define PIN_CS   13
#define PIN_SCK  14
#define PIN_MOSI 15
#define PIN_DC   12
#define PIN_RST  10

// Test delay constants
#define DELAY_1S 1000

// Initialize display class object
GC9107_TFT myTFT;

// Helper function to draw a rectangle outline
void draw_rectangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{
    myTFT.drawRectWH(x0, y0, x1 - x0 + 1, y1 - y0 + 1, color);
}

// Helper function to draw a cross in center
void draw_cross(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{
    uint16_t cx = (x0 + x1) / 2;
    uint16_t cy = (y0 + y1) / 2;
    int span = 20;

    myTFT.drawFastHLine(cx - span, cy, span * 2 + 1, color);
    myTFT.drawFastVLine(cx, cy - span, span * 2 + 1, color);
}

// Helper function to draw a string
void draw_string(uint16_t x, uint16_t y, const char* str, uint16_t color, uint16_t bg_color)
{
    myTFT.setTextColor(color, bg_color);
    myTFT.writeCharString(x, y, str);
}

int main(void)
{
    stdio_init_all();
    printf("GC9107 Display Test - Starting...\n");

    // Setup hardware SPI
    uint32_t TFT_SCLK_FREQ = 40000; // 40 MHz (40000 KHz)
    myTFT.TFTInitSPIType(TFT_SCLK_FREQ, SPI_PORT);

    // Setup GPIO
    myTFT.TFTsetupGPIO(PIN_RST, PIN_DC, PIN_CS, PIN_SCK, PIN_MOSI);

    // Screen Setup (128x128)
    myTFT.TFTInitScreen(128, 128, GC9107_TFT::GM_memory_base_e::MEMORY_BASE_GM_128x128, GC9107_TFT::MADCTL_FLAGS_t::RGB);

    // Set panel offset
    myTFT.TFTsetPanelOffset(0, 32);

    // Initialize screen
    myTFT.TFTGC9107Initialize();
    myTFT.TFTsetRotation(myTFT.Degrees_0);
    myTFT.setFont(font_default);

    printf("Display initialized. Running continuous test...\n");

    while (true)
    {
        // === Pattern 1: Solid colors ===
        myTFT.fillScreen(myTFT.C_WHITE);
        sleep_ms(DELAY_1S);

        myTFT.fillScreen(myTFT.C_BLUE);
        sleep_ms(DELAY_1S);

        myTFT.fillScreen(myTFT.C_GREEN);
        sleep_ms(DELAY_1S);

        myTFT.fillScreen(myTFT.C_RED);
        sleep_ms(DELAY_1S);

        myTFT.fillScreen(myTFT.C_BLACK);
        sleep_ms(DELAY_1S);

        // === Pattern 2: Rectangle with center cross ===
        myTFT.fillScreen(myTFT.C_WHITE);
        draw_rectangle(10, 10, 117, 117, myTFT.C_BLACK);  // Outer border
        draw_cross(10, 10, 117, 117, myTFT.C_RED);       // Red cross
        sleep_ms(DELAY_1S);

        myTFT.fillScreen(myTFT.C_BLACK);
        draw_rectangle(10, 10, 117, 117, myTFT.C_WHITE);  // Outer border
        draw_cross(10, 10, 117, 117, myTFT.C_GREEN);      // Green cross
        sleep_ms(DELAY_1S);

        myTFT.fillScreen(myTFT.C_BLUE);
        draw_rectangle(15, 15, 112, 112, myTFT.C_YELLOW); // Outer border
        draw_cross(15, 15, 112, 112, myTFT.C_MAGENTA);    // Cross
        sleep_ms(DELAY_1S);

        // === Pattern 3: Text display ===
        // White background with "GC9107" text
        myTFT.fillScreen(myTFT.C_WHITE);
        draw_string(35, 45, "GC9107", myTFT.C_BLACK, myTFT.C_WHITE);
        draw_rectangle(20, 30, 107, 97, myTFT.C_BLACK);  // Frame
        sleep_ms(DELAY_1S);

        // Blue background with "HELLO" text
        myTFT.fillScreen(myTFT.C_BLUE);
        draw_string(45, 50, "HELLO", myTFT.C_WHITE, myTFT.C_BLUE);
        draw_rectangle(20, 30, 107, 97, myTFT.C_WHITE);  // Frame
        sleep_ms(DELAY_1S);

        // Green background with "PICO" text
        myTFT.fillScreen(myTFT.C_GREEN);
        draw_string(45, 50, "PICO", myTFT.C_BLACK, myTFT.C_GREEN);
        draw_rectangle(20, 30, 107, 97, myTFT.C_BLACK);  // Frame
        sleep_ms(DELAY_1S);

        // Red background with "TEST" text
        myTFT.fillScreen(myTFT.C_RED);
        draw_string(45, 50, "TEST", myTFT.C_WHITE, myTFT.C_RED);
        draw_rectangle(20, 30, 107, 97, myTFT.C_WHITE);  // Frame
        sleep_ms(DELAY_1S);

        // === Pattern 4: Corner indicators ===
        myTFT.fillScreen(myTFT.C_BLACK);
        // Draw colored corners
        myTFT.fillRect(0, 0, 21, 21, myTFT.C_RED);         // Top-left
        myTFT.fillRect(107, 0, 21, 21, myTFT.C_GREEN);     // Top-right
        myTFT.fillRect(0, 107, 21, 21, myTFT.C_BLUE);      // Bottom-left
        myTFT.fillRect(107, 107, 21, 21, myTFT.C_WHITE);   // Bottom-right
        sleep_ms(DELAY_1S);

        // === Pattern 5: Checkerboard ===
        myTFT.fillScreen(myTFT.C_WHITE);
        for (int y = 0; y < 128; y += 8) {
            for (int x = 0; x < 128; x += 8) {
                if ((x / 8 + y / 8) % 2 == 0) {
                    myTFT.fillRect(x, y, 8, 8, myTFT.C_BLACK);
                }
            }
        }
        draw_string(40, 100, "CHECKER", myTFT.C_BLACK, myTFT.C_WHITE);
        sleep_ms(DELAY_1S);

        // === Pattern 6: Rainbow gradient lines ===
        myTFT.fillScreen(myTFT.C_BLACK);
        for (int i = 0; i < 128; i++) {
            uint16_t color;
            if (i < 42) color = myTFT.C_RED;
            else if (i < 85) color = myTFT.C_GREEN;
            else color = myTFT.C_BLUE;
            myTFT.drawFastHLine(0, i, 128, color);
        }
        draw_string(45, 110, "RAINBOW", myTFT.C_WHITE, myTFT.C_BLACK);
        sleep_ms(DELAY_1S);

        // === Pattern 7: Full border with center circle ===
        myTFT.fillScreen(myTFT.C_BLACK);
        draw_rectangle(0, 0, 127, 127, myTFT.C_WHITE);  // Full border
        myTFT.drawCircle(64, 64, 40, myTFT.C_RED);
        draw_string(45, 115, "PICO", myTFT.C_WHITE, myTFT.C_BLACK);
        sleep_ms(DELAY_1S);
    }
}
