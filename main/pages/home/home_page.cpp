#include "home_page.h"
#include "../../ui_engine.h"
#include "../../drawing.h"
#include "../../ui_types.h"
#include "../../fonts_pro.h" // Access to MyFontPro

// Helper to draw values dynamically with the new font
static void drawValueDynamic(int16_t x, int16_t y, int16_t val, uint16_t color) {
    char buf[10];
    snprintf(buf, 10, "%d%%", val);
    
    // Clear previous text area
    // Note: Adjust height/y based on your font size
    draw_fillRect(x, y - 12, 60, 16, COLOR_WHITE);
    
    // FIX: Use 'buf' here, not 'label'. And use MyFontPro.
    draw_GFXtext(x, y, buf, &MyFontPro, color); 
}

void screen_home_draw(void)
{
    // Clear content area
    draw_fillRect(0, CONTENT_Y, SCREEN_WIDTH, CONTENT_HEIGHT, 0xF7BE); // Off-white background

    const int16_t cardW = (SCREEN_WIDTH - 30) / 2;
    const int16_t cardH = 90;
    const int16_t margin = 10;

    // Row 1
    draw_card_modern(margin, CONTENT_Y + 20, cardW, cardH, "Moisture", 65, COLOR_CYAN);
    draw_card_modern(margin + cardW + 10, CONTENT_Y + 20, cardW, cardH, "Nitrogen", 42, COLOR_GREEN);

    // Row 2
    draw_card_modern(margin, CONTENT_Y + 20 + cardH + 15, cardW, cardH, "Phosphorus", 38, COLOR_ORANGE);
    draw_card_modern(margin + cardW + 10, CONTENT_Y + 20 + cardH + 15, cardW, cardH, "Potassium", 85, COLOR_MAGENTA);
}