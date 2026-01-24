/**
 * @file drawing.h
 * @brief Drawing Engine - Graphics Primitives and UI Elements
 *
 * Functions for drawing pixels, lines, rectangles, and UI components
 */

#ifndef DRAWING_H
#define DRAWING_H

#include <Arduino.h>
#include "config.h"
#include "ui_types.h"

// ===================================
// Basic Drawing Primitives
// ===================================

/**
 * @brief Fill the entire screen with a color
 * @param color RGB565 color
 */
void draw_fillScreen(uint16_t color);

/**
 * @brief Draw a single pixel
 * @param x X coordinate
 * @param y Y coordinate
 * @param color RGB565 color
 */
void draw_pixel(int16_t x, int16_t y, uint16_t color);

/**
 * @brief Draw a filled rectangle
 * @param x X coordinate
 * @param y Y coordinate
 * @param w Width
 * @param h Height
 * @param color RGB565 color
 */
void draw_fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

/**
 * @brief Draw a rectangle outline
 * @param x X coordinate
 * @param y Y coordinate
 * @param w Width
 * @param h Height
 * @param color RGB565 color
 */
void draw_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

/**
 * @brief Draw a line
 * @param x0 Start X coordinate
 * @param y0 Start Y coordinate
 * @param x1 End X coordinate
 * @param y1 End Y coordinate
 * @param color RGB565 color
 */
void draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);

/**
 * @brief Draw a horizontal line (optimized)
 * @param x Start X coordinate
 * @param y Y coordinate
 * @param w Width
 * @param color RGB565 color
 */
void draw_hLine(int16_t x, int16_t y, int16_t w, uint16_t color);

/**
 * @brief Draw a vertical line (optimized)
 * @param x X coordinate
 * @param y Start Y coordinate
 * @param h Height
 * @param color RGB565 color
 */
void draw_vLine(int16_t x, int16_t y, int16_t h, uint16_t color);

// ===================================
// UI Component Drawing
// ===================================

/**
 * @brief Draw a card with header
 * @param x X coordinate
 * @param y Y coordinate
 * @param w Width
 * @param h Height
 * @param label Card label (not rendered, needs font)
 * @param value Display value
 * @param color Accent color
 */
void draw_card(int16_t x, int16_t y, int16_t w, int16_t h,
               const char *label, int16_t value, uint16_t color);

/**
 * @brief Draw a button
 * @param btn Pointer to button structure
 */
void draw_button(UIButton *btn);

/**
 * @brief Draw a simple icon (colored square placeholder)
 * @param x X coordinate
 * @param y Y coordinate
 * @param size Icon size
 * @param color Icon color
 */
void draw_icon(int16_t x, int16_t y, int16_t size, uint16_t color);

/**
 * @brief Draw a monochrome bitmap icon from PROGMEM
 * @param x X coordinate
 * @param y Y coordinate
 * @param bitmap Pointer to bitmap data in PROGMEM
 * @param w Width in pixels
 * @param h Height in pixels
 * @param color Foreground color
 */
void draw_iconBitmap(int16_t x, int16_t y, const unsigned char *bitmap,
                     int16_t w, int16_t h, uint16_t color);

/**
 * @brief Draw a progress bar
 * @param x X coordinate
 * @param y Y coordinate
 * @param w Width
 * @param h Height
 * @param percent Fill percentage (0-100)
 * @param fgColor Foreground color
 * @param bgColor Background color
 */
void draw_progressBar(int16_t x, int16_t y, int16_t w, int16_t h,
                      uint8_t percent, uint16_t fgColor, uint16_t bgColor);

// ===================================
// Status Bar Components
// ===================================

/**
 * @brief Draw GSM signal strength indicator
 * @param x X coordinate
 * @param y Y coordinate
 * @param signal Signal strength (0-100)
 */
void draw_gsmSignal(int16_t x, int16_t y, uint8_t signal);

/**
 * @brief Draw battery indicator
 * @param x X coordinate
 * @param y Y coordinate
 * @param level Battery level (0-100)
 */
void draw_battery(int16_t x, int16_t y, uint8_t level);

/**
 * @brief Draw GPS lock indicator
 * @param x X coordinate
 * @param y Y coordinate
 * @param locked GPS lock status
 */
void draw_gpsIndicator(int16_t x, int16_t y, bool locked);

#endif // DRAWING_H
