/**
 * @file drawing.cpp
 * @brief Drawing Engine Implementation
 */

#include "drawing.h"
#include "tft_driver.h"

// ===================================
// Basic Drawing Primitives
// ===================================

void draw_fillScreen(uint16_t color)
{
    tft_setWindow(0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1);
    tft_beginWrite();
    for (uint32_t i = 0; i < (uint32_t)SCREEN_WIDTH * SCREEN_HEIGHT; i++)
    {
        tft_writeColor(color);
    }
    tft_endWrite();
}

void draw_pixel(int16_t x, int16_t y, uint16_t color)
{
    if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT)
    {
        return;
    }
    tft_setWindow(x, y, x, y);
    tft_writeData16(color);
}

void draw_fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    // Bounds checking and clipping
    if (x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT)
        return;
    if (w <= 0 || h <= 0)
        return;

    if (x < 0)
    {
        w += x;
        x = 0;
    }
    if (y < 0)
    {
        h += y;
        y = 0;
    }
    if (x + w > SCREEN_WIDTH)
        w = SCREEN_WIDTH - x;
    if (y + h > SCREEN_HEIGHT)
        h = SCREEN_HEIGHT - y;

    if (w <= 0 || h <= 0)
        return;

    tft_setWindow(x, y, x + w - 1, y + h - 1);
    tft_beginWrite();
    for (int32_t i = 0; i < (int32_t)w * h; i++)
    {
        tft_writeColor(color);
    }
    tft_endWrite();
}

void draw_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    draw_hLine(x, y, w, color);         // Top
    draw_hLine(x, y + h - 1, w, color); // Bottom
    draw_vLine(x, y, h, color);         // Left
    draw_vLine(x + w - 1, y, h, color); // Right
}

void draw_hLine(int16_t x, int16_t y, int16_t w, uint16_t color)
{
    draw_fillRect(x, y, w, 1, color);
}

void draw_vLine(int16_t x, int16_t y, int16_t h, uint16_t color)
{
    draw_fillRect(x, y, 1, h, color);
}

void draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{
    // Bresenham's line algorithm
    int16_t dx = abs(x1 - x0);
    int16_t dy = abs(y1 - y0);
    int16_t sx = (x0 < x1) ? 1 : -1;
    int16_t sy = (y0 < y1) ? 1 : -1;
    int16_t err = dx - dy;

    while (true)
    {
        draw_pixel(x0, y0, color);
        if (x0 == x1 && y0 == y1)
            break;

        int16_t e2 = 2 * err;
        if (e2 > -dy)
        {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}

// ===================================
// UI Component Drawing
// ===================================

void draw_card(int16_t x, int16_t y, int16_t w, int16_t h,
               const char *label, int16_t value, uint16_t color)
{
    // Card border
    draw_fillRect(x, y, w, h, color);
    // Card body
    draw_fillRect(x + 2, y + 2, w - 4, h - 4, COLOR_WHITE);
    // Card header
    draw_fillRect(x + 5, y + 5, w - 10, 20, color);
    // Value area
    draw_fillRect(x + 10, y + 30, w - 20, 25, COLOR_LIGHTGRAY);
}

void draw_button(UIButton *btn)
{
    if (btn == NULL || !btn->visible)
    {
        return;
    }

    // Button border
    draw_fillRect(btn->x, btn->y, btn->w, btn->h, btn->color);
    // Button face
    draw_fillRect(btn->x + 2, btn->y + 2, btn->w - 4, btn->h - 4, COLOR_WHITE);
}

void draw_icon(int16_t x, int16_t y, int16_t size, uint16_t color)
{
    draw_fillRect(x, y, size, size, color);
}

void draw_progressBar(int16_t x, int16_t y, int16_t w, int16_t h,
                      uint8_t percent, uint16_t fgColor, uint16_t bgColor)
{
    if (percent > 100)
        percent = 100;

    // Background
    draw_fillRect(x, y, w, h, bgColor);

    // Filled portion
    int16_t fillWidth = (w * percent) / 100;
    if (fillWidth > 0)
    {
        draw_fillRect(x, y, fillWidth, h, fgColor);
    }
}

// ===================================
// Status Bar Components
// ===================================

void draw_gsmSignal(int16_t x, int16_t y, uint8_t signal)
{
    int bars = signal / 25; // 0-4 bars

    for (int i = 0; i < 4; i++)
    {
        uint16_t color = (i < bars) ? COLOR_GREEN : COLOR_GRAY;
        int16_t barHeight = 5 + i * 3;
        int16_t barY = y + (12 - barHeight);
        draw_fillRect(x + i * 6, barY, 4, barHeight, color);
    }
}

void draw_battery(int16_t x, int16_t y, uint8_t level)
{
    // Battery outline
    draw_fillRect(x, y, 20, 10, COLOR_WHITE);
    // Battery terminal
    draw_fillRect(x + 20, y + 3, 2, 4, COLOR_WHITE);

    // Fill level
    int16_t fillWidth = (level * 18) / 100;
    uint16_t fillColor = (level > 20) ? COLOR_GREEN : COLOR_RED;

    if (fillWidth > 0)
    {
        draw_fillRect(x + 1, y + 1, fillWidth, 8, fillColor);
    }
}

void draw_gpsIndicator(int16_t x, int16_t y, bool locked)
{
    uint16_t color = locked ? COLOR_GREEN : COLOR_GRAY;
    draw_fillRect(x, y, 8, 8, color);
}
