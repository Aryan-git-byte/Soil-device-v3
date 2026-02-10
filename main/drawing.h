#ifndef DRAWING_H
#define DRAWING_H

#include <Arduino.h>
#include "config.h"
#include "ui_types.h"
#include "fonts_pro.h" // Include our new font definition

// ===================================
// Basic Drawing Primitives
// ===================================

void draw_fillScreen(uint16_t color);
void draw_pixel(int16_t x, int16_t y, uint16_t color);
void draw_fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void draw_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
void draw_hLine(int16_t x, int16_t y, int16_t w, uint16_t color);
void draw_vLine(int16_t x, int16_t y, int16_t h, uint16_t color);

// ===================================
// Advanced Shapes (New)
// ===================================

/**
 * @brief Draw a filled circle (helper for rounded rects)
 */
void draw_filledCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);

/**
 * @brief Draw a rounded rectangle with anti-aliasing simulation
 * @param x Top-left X
 * @param y Top-left Y
 * @param w Width
 * @param h Height
 * @param r Corner radius
 * @param color Fill color
 */
void draw_roundedRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);

// ===================================
// Advanced Text (New)
// ===================================

/**
 * @brief Draw text using a proportional GFX font
 * @param x Left X coordinate of the text baseline
 * @param y Baseline Y coordinate
 * @param str The string to draw
 * @param font Pointer to the GFXfont structure
 * @param color Text color
 */
void draw_GFXtext(int16_t x, int16_t y, const char *str, const GFXfont *font, uint16_t color);

/**
 * @brief Helper to measure text width
 */
int16_t get_GFXtextWidth(const char *str, const GFXfont *font);

// ===================================
// UI Component Drawing
// ===================================

// Legacy (keep for other screens)
void draw_card(int16_t x, int16_t y, int16_t w, int16_t h, const char *label, int16_t value, uint16_t color);

// MODERN (Use this for home screen)
void draw_card_modern(int16_t x, int16_t y, int16_t w, int16_t h, const char *label, int16_t value, uint16_t color);

void draw_button(UIButton *btn);
void draw_icon(int16_t x, int16_t y, int16_t size, uint16_t color);
void draw_iconBitmap(int16_t x, int16_t y, const unsigned char *bitmap, int16_t w, int16_t h, uint16_t color);
void draw_progressBar(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t percent, uint16_t fgColor, uint16_t bgColor);

// ===================================
// Status Bar Components
// ===================================

void draw_gsmSignal(int16_t x, int16_t y, uint8_t signal);
void draw_battery(int16_t x, int16_t y, uint8_t level);
void draw_gpsIndicator(int16_t x, int16_t y, bool locked);

#endif // DRAWING_H