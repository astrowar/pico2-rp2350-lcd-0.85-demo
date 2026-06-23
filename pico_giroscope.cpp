/*!
    @file    pico_giroscope.cpp
    @author  Adapted to use displaylib_16bit_PICO library
    @brief   Display gyroscope and accelerometer data on GC9107 display
    @details Uses QMI8658A 6-axis IMU (gyroscope + accelerometer) via I2C
    @note    GPIO pins for QMI8658A: SDA=20, SCL=21
*/

#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "displaylib_16/gc9107.hpp"

// === SPI and GPIO defines for GC9107 Display ===
#define SPI_PORT    spi1
#define PIN_CS      13
#define PIN_SCK     14
#define PIN_MOSI    15
#define PIN_DC      12
#define PIN_RST     10
#define PIN_WS2812  25  // WS2812 RGB LED data pin

// === QMI8658A I2C defines ===
#define QMI8658A_I2C_ADDR 0x6B  // QMI8658A I2C address (7-bit)
#define QMI8658A_I2C_PORT i2c0
#define PIN_I2C_SDA 20
#define PIN_I2C_SCL 21

// === Display Dimensions ===
#define DISPLAY_WIDTH  128
#define DISPLAY_HEIGHT 128

// === Timing delays ===
#define DELAY_20MS   20
#define DELAY_500MS  500

// Initialize display class object
GC9107_TFT myTFT;

// === Helper Wrappers matching previous behavior but using myTFT ===
void fill_screen(uint16_t color) {
    myTFT.fillScreen(color);
}

void draw_circle(uint16_t x, uint16_t y, uint16_t radius, uint16_t color) {
    myTFT.drawCircle(x, y, radius, color);
}

void draw_cross(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) {
    uint16_t cx = (x0 + x1) / 2;
    uint16_t cy = (y0 + y1) / 2;
    int span = 20;
    myTFT.drawFastHLine(cx - span, cy, span * 2 + 1, color);
    myTFT.drawFastVLine(cx, cy - span, span * 2 + 1, color);
}

void draw_string(uint16_t x, uint16_t y, const char* str, uint16_t color, uint16_t bg_color) {
    myTFT.setTextColor(color, bg_color);
    myTFT.writeCharString(x, y, str);
}

// === Function prototypes for WS2812 RGB LED ===
void ws2812_init(void);
void ws2812_set_color(uint8_t r, uint8_t g, uint8_t b);
void ws2812_off(void);

// === Function prototypes for QMI8658A IMU ===
void i2c_init_display(void);
bool qmi8658a_init(void);
bool qmi8658a_read_raw(float* ax, float* ay, float* az,
                       float* gx, float* gy, float* gz);
void qmi8658a_get_orientation(float* roll, float* pitch);

// === Function prototypes for IMU Display ===
void display_imu_data(float ax, float ay, float az,
                      float gx, float gy, float gz,
                      float roll, float pitch);
void qmi8658a_update_orientation(float* roll, float* pitch, float dt);
void draw_level_indicator(uint16_t cx, uint16_t cy, uint16_t radius, float roll, float pitch);

// === Filter constants ===
static const float G_ALPHA = 0.80f;  // Filter coefficient - reduced for stability
static const float SAMPLE_DT = 0.02f;  // 20ms sampling period
static const float PI = 3.14159265359f;

// === MAIN ===
int main(void)
{
    stdio_init_all();
    printf("Pico Gyroscope Display - Starting...\n");

    // Setup hardware SPI for TFT
    uint32_t TFT_SCLK_FREQ = 40000; // 40 MHz (40000 KHz)
    myTFT.TFTInitSPIType(TFT_SCLK_FREQ, SPI_PORT);

    // Setup GPIO for TFT
    myTFT.TFTsetupGPIO(PIN_RST, PIN_DC, PIN_CS, PIN_SCK, PIN_MOSI);

    // Screen Setup (128x128)
    myTFT.TFTInitScreen(128, 128, GC9107_TFT::GM_memory_base_e::MEMORY_BASE_GM_128x128, GC9107_TFT::MADCTL_FLAGS_t::RGB);

    // Set panel offset
    myTFT.TFTsetPanelOffset(0, 32);

    // Initialize screen
    myTFT.TFTGC9107Initialize();
    myTFT.TFTsetRotation(myTFT.Degrees_0);
    myTFT.setFont(font_default);

    // Initialize WS2812 RGB LED - keep it OFF
    ws2812_init();
    ws2812_off();

    // Initialize I2C for QMI8658A
    i2c_init_display();

    // Initialize QMI8658A IMU
    if (!qmi8658a_init()) {
        printf("ERROR: QMI8658A initialization failed!\n");
    } else {
        printf("QMI8658A initialized successfully!\n");
    }

    // Wait for sensor to be ready
    sleep_ms(DELAY_500MS);
    printf("Starting gyroscope display loop...\n");

    // Main loop - continuous display of gyroscope data (50Hz)
    uint32_t last_time = time_us_32();
    int loop_count = 0;
    float last_roll = 0, last_pitch = 0;
    while (true) {
        float roll, pitch;

        // Update orientation using fixed sample period for stability
        qmi8658a_update_orientation(&roll, &pitch, SAMPLE_DT);

        // Display data only if values changed significantly (reduces flicker)
        if (abs(roll - last_roll) > 0.5f || abs(pitch - last_pitch) > 0.5f) {
            display_imu_data(0, 0, 0, 0, 0, 0, roll, pitch);
            last_roll = roll;
            last_pitch = pitch;
        }

        // Debug: Print every 10 loops
        loop_count++;
        if (loop_count % 10 == 0) {
            printf("Loop %d: roll=%.2f pitch=%.2f\n", loop_count, roll, pitch);
        }

        sleep_ms(DELAY_20MS);
    }
}

// === Level Indicator Display Function ===
void draw_level_indicator(uint16_t cx, uint16_t cy, uint16_t radius, float roll, float pitch)
{
    // Fixed central circle (container)
    draw_circle(cx, cy, radius, myTFT.C_WHITE);

    // Small bubble circle moving based on tilt
    int16_t move_x = static_cast<int16_t>(roll * 0.8f);
    int16_t move_y = static_cast<int16_t>(pitch * 0.8f);

    // Limit bubble position inside central circle
    int16_t dist = static_cast<int16_t>(sqrtf(move_x * move_x + move_y * move_y));
    int16_t max_move = radius - 5;
    if (dist > max_move && dist != 0) {
        float scale = static_cast<float>(max_move) / static_cast<float>(dist);
        move_x = static_cast<int16_t>(move_x * scale);
        move_y = static_cast<int16_t>(move_y * scale);
    }

    // Draw level bubble circle
    draw_circle(cx + move_x, cy + move_y, radius / 4, myTFT.C_WHITE);
}

void display_imu_data(float ax, float ay, float az,
                      float gx, float gy, float gz,
                      float roll, float pitch)
{
    // Clear screen
    fill_screen(myTFT.C_BLACK);

    // Title
    draw_string(30, 10, "LEVEL METER", myTFT.C_YELLOW, myTFT.C_BLACK);

    // Draw level indicator in center (64, 64) with radius 40
    draw_level_indicator(64, 64, 40, roll, pitch);

    // Draw crosshair at center
    draw_cross(24, 24, 103, 103, myTFT.C_WHITE);

    // Show orientation values at bottom
    char buf[20];
    draw_string(15, 105, "ROLL:", myTFT.C_GREEN, myTFT.C_BLACK);
    snprintf(buf, sizeof(buf), "%6.1f", roll);
    draw_string(55, 105, buf, myTFT.C_WHITE, myTFT.C_BLACK);

    draw_string(15, 115, "PITCH:", myTFT.C_GREEN, myTFT.C_BLACK);
    snprintf(buf, sizeof(buf), "%6.1f", pitch);
    draw_string(55, 115, buf, myTFT.C_WHITE, myTFT.C_BLACK);
}

// === I2C and QMI8658A Functions ===

void i2c_init_display(void)
{
    i2c_init(QMI8658A_I2C_PORT, 400000);
    gpio_set_function(PIN_I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(PIN_I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(PIN_I2C_SDA);
    gpio_pull_up(PIN_I2C_SCL);
}

bool qmi8658a_init(void)
{
    // Check if QMI8658A is present (WHO_AM_I register at 0x00)
    uint8_t rx_buf[1];
    uint8_t addr = 0x00;
    i2c_write_blocking(QMI8658A_I2C_PORT, QMI8658A_I2C_ADDR, &addr, 1, true);
    i2c_read_blocking(QMI8658A_I2C_PORT, QMI8658A_I2C_ADDR, rx_buf, 1, false);
    printf("QMI8658A Device ID: 0x%02X\n", rx_buf[0]);

    if (rx_buf[0] != 0x05) {
        printf("ERROR: QMI8658A not detected! Expected 0x05, got 0x%02X\n", rx_buf[0]);
        return false;
    }

    // Configure QMI8658A
    addr = 0x02;
    uint8_t ctrl1 = 0x40;
    i2c_write_blocking(QMI8658A_I2C_PORT, QMI8658A_I2C_ADDR, &addr, 1, true);
    i2c_write_blocking(QMI8658A_I2C_PORT, QMI8658A_I2C_ADDR, &ctrl1, 1, false);

    addr = 0x03;
    uint8_t ctrl2 = (0b001 << 4) | 0b0110;  // ±4g, 125Hz
    i2c_write_blocking(QMI8658A_I2C_PORT, QMI8658A_I2C_ADDR, &addr, 1, true);
    i2c_write_blocking(QMI8658A_I2C_PORT, QMI8658A_I2C_ADDR, &ctrl2, 1, false);

    addr = 0x04;
    uint8_t ctrl3 = (0b101 << 4) | 0b0110;  // ±512dps, 125Hz
    i2c_write_blocking(QMI8658A_I2C_PORT, QMI8658A_I2C_ADDR, &addr, 1, true);
    i2c_write_blocking(QMI8658A_I2C_PORT, QMI8658A_I2C_ADDR, &ctrl3, 1, false);

    addr = 0x08;
    uint8_t ctrl7 = 0x03;
    i2c_write_blocking(QMI8658A_I2C_PORT, QMI8658A_I2C_ADDR, &addr, 1, true);
    i2c_write_blocking(QMI8658A_I2C_PORT, QMI8658A_I2C_ADDR, &ctrl7, 1, false);

    // Wait for sensor to stabilize
    sleep_ms(DELAY_500MS);
    return true;
}

bool qmi8658a_read_raw(float* ax, float* ay, float* az,
                       float* gx, float* gy, float* gz)
{
    uint8_t buf[12];
    uint8_t addr = 0x35;  // AX_L register

    i2c_write_blocking(QMI8658A_I2C_PORT, QMI8658A_I2C_ADDR, &addr, 1, true);
    if (!i2c_read_blocking(QMI8658A_I2C_PORT, QMI8658A_I2C_ADDR, buf, 12, false)) {
        printf("ERROR: I2C read failed!\n");
        return false;
    }

    int16_t raw_ax = (int16_t)((buf[1] << 8) | buf[0]);
    int16_t raw_ay = (int16_t)((buf[3] << 8) | buf[2]);
    int16_t raw_az = (int16_t)((buf[5] << 8) | buf[4]);
    int16_t raw_gx = (int16_t)((buf[7] << 8) | buf[6]);
    int16_t raw_gy = (int16_t)((buf[9] << 8) | buf[8]);
    int16_t raw_gz = (int16_t)((buf[11] << 8) | buf[10]);

    *ax = (float)raw_ax * 4.0f / 32768.0f;
    *ay = (float)raw_ay * 4.0f / 32768.0f;
    *az = (float)raw_az * 4.0f / 32768.0f;
    *gx = (float)raw_gx * 512.0f / 32768.0f;
    *gy = (float)raw_gy * 512.0f / 32768.0f;
    *gz = (float)raw_gz * 512.0f / 32768.0f;

    return true;
}

// Complementary filter state
static float g_roll = 0.0f;
static float g_pitch = 0.0f;

void qmi8658a_update_orientation(float* roll, float* pitch, float dt)
{
    float ax, ay, az, gx, gy, gz;
    qmi8658a_read_raw(&ax, &ay, &az, &gx, &gy, &gz);

    float acc_pitch = -atan2f(ay, az) * 180.0f / PI;
    float acc_roll = -atan2f(-ax, sqrtf(ay * ay + az * az)) * 180.0f / PI;

    g_roll += gy * SAMPLE_DT;
    g_pitch += gx * SAMPLE_DT;

    g_roll = G_ALPHA * g_roll + (1.0f - G_ALPHA) * acc_roll;
    g_pitch = G_ALPHA * g_pitch + (1.0f - G_ALPHA) * acc_pitch;

    *roll = g_roll;
    *pitch = g_pitch;
}

// === WS2812 RGB LED Functions ===
void ws2812_init(void)
{
    gpio_init(PIN_WS2812);
    gpio_set_dir(PIN_WS2812, GPIO_OUT);
    gpio_put(PIN_WS2812, 0);
    gpio_set_function(PIN_WS2812, GPIO_FUNC_SIO);
}

static void ws2812_send_byte(uint8_t data)
{
    for (int bit = 7; bit >= 0; bit--) {
        if ((data & (1 << bit))) {
            gpio_put(PIN_WS2812, 1);
            sleep_us(1);
            gpio_put(PIN_WS2812, 0);
            sleep_us(2);
        } else {
            gpio_put(PIN_WS2812, 1);
            sleep_us(2);
            gpio_put(PIN_WS2812, 0);
            sleep_us(1);
        }
    }
}

void ws2812_set_color(uint8_t r, uint8_t g, uint8_t b)
{
    ws2812_send_byte(g);
    ws2812_send_byte(r);
    ws2812_send_byte(b);

    gpio_put(PIN_WS2812, 0);
    sleep_us(100);
}

void ws2812_off(void)
{
    gpio_put(PIN_WS2812, 0);
    sleep_us(280);
}
