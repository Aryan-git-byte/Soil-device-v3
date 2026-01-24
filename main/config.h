/**
 * @file config.h
 * @brief Configuration constants for the UI Engine
 *
 * Pin definitions, screen dimensions, colors, and calibration constants
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ===================================
// SPI Pin Definitions (Shared)
// ===================================
#define SPI_MOSI 13 // Shared: TFT SDI/MOSI and Touch T_DIN
#define SPI_MISO 12 // Shared: TFT SDO/MISO and Touch T_DO
#define SPI_SCK 11  // Shared: TFT SCK and Touch T_CLK

// ===================================
// TFT Pin Definitions
// ===================================
#define TFT_CS 10
#define TFT_RST 8
#define TFT_DC 7

// ===================================
// Touch Pin Definitions
// ===================================
#define T_CS A2
#define T_IRQ A1 // Optional interrupt pin

// ===================================
// SD Card Pin Definition
// ===================================
#define SD_CS 4

// ===================================
// Screen Dimensions
// ===================================
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320

// ===================================
// UI Layout Constants
// ===================================
#define NAVBAR_HEIGHT 50
#define NAVBAR_Y (SCREEN_HEIGHT - NAVBAR_HEIGHT)
#define HEADER_HEIGHT 30
#define STATUS_HEIGHT 0
#define CONTENT_Y (HEADER_HEIGHT + STATUS_HEIGHT)
#define CONTENT_HEIGHT (NAVBAR_Y - CONTENT_Y)

// ===================================
// ILI9341 Commands
// ===================================
#define ILI9341_SWRESET 0x01
#define ILI9341_SLPOUT 0x11
#define ILI9341_DISPON 0x29
#define ILI9341_CASET 0x2A
#define ILI9341_PASET 0x2B
#define ILI9341_RAMWR 0x2C
#define ILI9341_MADCTL 0x36
#define ILI9341_PIXFMT 0x3A
#define ILI9341_FRMCTR1 0xB1
#define ILI9341_PWCTR1 0xC0
#define ILI9341_PWCTR2 0xC1
#define ILI9341_VMCTR1 0xC5
#define ILI9341_VMCTR2 0xC7

// ===================================
// XPT2046 Touch Commands
// ===================================
#define XPT2046_CMD_X 0xD0
#define XPT2046_CMD_Y 0x90
#define XPT2046_CMD_Z1 0xB0
#define XPT2046_CMD_Z2 0xC0

// ===================================
// Colors (RGB565 Format)
// ===================================
#define COLOR_BLACK 0x0000
#define COLOR_RED 0xF800
#define COLOR_GREEN 0x07E0
#define COLOR_BLUE 0x001F
#define COLOR_CYAN 0x07FF
#define COLOR_MAGENTA 0xF81F
#define COLOR_YELLOW 0xFFE0
#define COLOR_WHITE 0xFFFF
#define COLOR_ORANGE 0xFD20
#define COLOR_GRAY 0x8410
#define COLOR_DARKGRAY 0x4208
#define COLOR_LIGHTGRAY 0xC618
#define COLOR_DARKGREEN 0x03E0

// ===================================
// Touch Calibration Constants
// ===================================
#define TS_MINX 414
#define TS_MINY 311
#define TS_MAXX 3583
#define TS_MAXY 3713
#define PRESSURE_THRESHOLD 400

// ===================================
// UI Engine Limits
// ===================================
#define MAX_BUTTONS 12
#define MAX_VALUES 8
#define MAX_ALERT_LEN 32
#define TOUCH_DEBOUNCE_MS 200
#define ALERT_TIMEOUT_MS 3000
#define SENSOR_UPDATE_MS 2000

// ===================================
// File Browser Settings
// ===================================
#define MAX_FILES_DISPLAY 20
#define FILE_ITEM_HEIGHT 35
#define SCROLLBAR_WIDTH 8

#endif // CONFIG_H
