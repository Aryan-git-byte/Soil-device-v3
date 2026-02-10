#include "drawing.h"
#include "tft_driver.h"
#include <string.h> 
#include <stdlib.h> // For abs()
#include "fonts_pro.h" // Ensure this is included

// ===================================
// Basic Drawing Primitives
// ===================================

void draw_fillScreen(uint16_t color) {
    tft_setWindow(0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1);
    tft_beginWrite();
    for (uint32_t i = 0; i < (uint32_t)SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        tft_writeColor(color);
    }
    tft_endWrite();
}

void draw_pixel(int16_t x, int16_t y, uint16_t color) {
    if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT) return;
    tft_setWindow(x, y, x, y);
    tft_writeData16(color);
}

void draw_fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT) return;
    if (w <= 0 || h <= 0) return;
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > SCREEN_WIDTH) w = SCREEN_WIDTH - x;
    if (y + h > SCREEN_HEIGHT) h = SCREEN_HEIGHT - y;
    if (w <= 0 || h <= 0) return;

    tft_setWindow(x, y, x + w - 1, y + h - 1);
    tft_beginWrite();
    for (int32_t i = 0; i < (int32_t)w * h; i++) {
        tft_writeColor(color);
    }
    tft_endWrite();
}

void draw_hLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
    draw_fillRect(x, y, w, 1, color);
}

void draw_vLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
    draw_fillRect(x, y, 1, h, color);
}

void draw_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    draw_hLine(x, y, w, color);
    draw_hLine(x, y + h - 1, w, color);
    draw_vLine(x, y, h, color);
    draw_vLine(x + w - 1, y, h, color);
}

void draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    int16_t dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int16_t dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int16_t err = dx + dy, e2;
    while (1) {
        draw_pixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

// ===================================
// Advanced Shapes (Rounded Rects)
// ===================================

static void draw_filledCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, int16_t delta, uint16_t color) {
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        if (cornername & 0x1) { // Left corners
            draw_vLine(x0 - x, y0 - y, 2 * y + 1 + delta, color);
            draw_vLine(x0 - y, y0 - x, 2 * x + 1 + delta, color);
        }
        if (cornername & 0x2) { // Right corners
            draw_vLine(x0 + x, y0 - y, 2 * y + 1 + delta, color);
            draw_vLine(x0 + y, y0 - x, 2 * x + 1 + delta, color);
        }
    }
}

void draw_filledCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
    draw_vLine(x0, y0 - r, 2 * r + 1, color);
    draw_filledCircleHelper(x0, y0, r, 3, 0, color);
}

void draw_roundedRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    draw_fillRect(x + r, y, w - 2 * r, h, color); // Center
    draw_fillRect(x, y + r, r, h - 2 * r, color); // Left
    draw_fillRect(x + w - r, y + r, r, h - 2 * r, color); // Right
    draw_filledCircleHelper(x + r, y + r, r, 1, h - 2 * r - 1, color); // Left side corners
    draw_filledCircleHelper(x + w - r - 1, y + r, r, 2, h - 2 * r - 1, color); // Right side corners
}

// ===================================
// Advanced Text (Proportional Fonts)
// ===================================

void draw_GFXtext(int16_t x, int16_t y, const char *str, const GFXfont *font, uint16_t color) {
    int16_t cursor_x = x;
    uint8_t first = pgm_read_byte(&font->first);
    uint8_t last = pgm_read_byte(&font->last);
    
    while (*str) {
        uint8_t c = *str++;
        if (c < first || c > last) continue;
        
        GFXglyph *glyph = &(((GFXglyph *)pgm_read_ptr(&font->glyph))[c - first]);
        uint8_t *bitmap = (uint8_t *)pgm_read_ptr(&font->bitmap);

        uint16_t bo = pgm_read_word(&glyph->bitmapOffset);
        uint8_t  w  = pgm_read_byte(&glyph->width);
        uint8_t  h  = pgm_read_byte(&glyph->height);
        int8_t   xo = (int8_t)pgm_read_byte(&glyph->xOffset);
        int8_t   yo = (int8_t)pgm_read_byte(&glyph->yOffset);
        uint8_t  xa = pgm_read_byte(&glyph->xAdvance);
        
        // --- FIXED BITMAP READER LOOP ---
        uint16_t bit_idx = 0;
        uint8_t  current_byte = 0;

        for (int yy = 0; yy < h; yy++) {
            for (int xx = 0; xx < w; xx++) {
                if ((bit_idx & 7) == 0) {
                    current_byte = pgm_read_byte(&bitmap[bo++]);
                }
                
                if (current_byte & 0x80) {
                    draw_pixel(cursor_x + xo + xx, y + yo + yy, color);
                }
                current_byte <<= 1;
                bit_idx++;
            }
        }
        cursor_x += xa;
    }
}

int16_t get_GFXtextWidth(const char *str, const GFXfont *font) {
    int16_t w = 0;
    uint8_t first = pgm_read_byte(&font->first);
    
    while (*str) {
        uint8_t c = *str++;
        if (c >= first && c <= pgm_read_byte(&font->last)) {
            GFXglyph *glyph = &(((GFXglyph *)pgm_read_ptr(&font->glyph))[c - first]);
            w += pgm_read_byte(&glyph->xAdvance);
        }
    }
    return w;
}

// ===================================
// UI Component Drawing
// ===================================

void draw_card_modern(int16_t x, int16_t y, int16_t w, int16_t h, const char *label, int16_t value, uint16_t color)
{
    // 1. Shadow (Simple offset rect)
    draw_roundedRect(x + 4, y + 4, w, h, 10, COLOR_LIGHTGRAY);
    
    // 2. Main Card Body
    draw_roundedRect(x, y, w, h, 10, COLOR_WHITE);
    
    // 3. Colored Header Strip
    draw_roundedRect(x, y, w, 25, 10, color);
    draw_fillRect(x, y + 15, w, 10, color); // Flatten bottom of header
    
    // 4. Label (using new Font!)
    draw_GFXtext(x + 10, y + 18, label, &MyFontPro, COLOR_WHITE); // <--- FIXED: uses MyFontPro
    
    // 5. Value
    char valStr[10];
    snprintf(valStr, 10, "%d%%", value);
    
    // Draw value big and centered
    int16_t textW = get_GFXtextWidth(valStr, &MyFontPro); // <--- FIXED: uses MyFontPro
    int16_t textX = x + (w - textW) / 2;
    draw_GFXtext(textX, y + 60, valStr, &MyFontPro, COLOR_BLACK); // <--- FIXED: uses MyFontPro
}

void draw_card(int16_t x, int16_t y, int16_t w, int16_t h, const char *label, int16_t value, uint16_t color) {
    draw_card_modern(x, y, w, h, label, value, color);
}

void draw_button(UIButton *btn) {
    if (!btn || !btn->visible) return;
    draw_fillRect(btn->x, btn->y, btn->w, btn->h, btn->color);
    draw_fillRect(btn->x + 2, btn->y + 2, btn->w - 4, btn->h - 4, COLOR_WHITE);
}

void draw_icon(int16_t x, int16_t y, int16_t size, uint16_t color) {
    draw_fillRect(x, y, size, size, color);
}

void draw_iconBitmap(int16_t x, int16_t y, const unsigned char *bitmap, int16_t w, int16_t h, uint16_t color) {
    int16_t byteWidth = (w + 7) / 8;
    uint8_t byte = 0;
    for (int16_t j = 0; j < h; j++) {
        for (int16_t i = 0; i < w; i++) {
            if (i & 7) byte <<= 1;
            else byte = pgm_read_byte(&bitmap[j * byteWidth + i / 8]);
            if (byte & 0x80) draw_pixel(x + i, y + j, color);
        }
    }
}

void draw_progressBar(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t percent, uint16_t fgColor, uint16_t bgColor) {
    if (percent > 100) percent = 100;
    draw_fillRect(x, y, w, h, bgColor);
    int16_t fillWidth = (w * percent) / 100;
    if (fillWidth > 0) draw_fillRect(x, y, fillWidth, h, fgColor);
}

// ===================================
// Status Bar Components
// ===================================

void draw_gsmSignal(int16_t x, int16_t y, uint8_t signal) {
    int bars = signal / 25;
    for (int i = 0; i < 4; i++) {
        uint16_t color = (i < bars) ? COLOR_GREEN : COLOR_GRAY;
        int16_t barHeight = 5 + i * 3;
        draw_fillRect(x + i * 6, y + (12 - barHeight), 4, barHeight, color);
    }
}

void draw_battery(int16_t x, int16_t y, uint8_t level) {
    draw_fillRect(x, y, 20, 10, COLOR_WHITE);
    draw_fillRect(x + 20, y + 3, 2, 4, COLOR_WHITE);
    int16_t fillWidth = (level * 18) / 100;
    uint16_t fillColor = (level > 20) ? COLOR_GREEN : COLOR_RED;
    if (fillWidth > 0) draw_fillRect(x + 1, y + 1, fillWidth, 8, fillColor);
}

void draw_gpsIndicator(int16_t x, int16_t y, bool locked) {
    uint16_t color = locked ? COLOR_GREEN : COLOR_GRAY;
    draw_fillRect(x, y, 8, 8, color);
}