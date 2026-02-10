#include "home_page.h"
#include "../../ui_engine.h" // Go up two levels to find these
#include "../../drawing.h"
#include "../../ui_types.h"

void screen_home_draw(void)
{
    // Clear content area
    draw_fillRect(0, CONTENT_Y, SCREEN_WIDTH, CONTENT_HEIGHT, COLOR_WHITE);

    // Calculate card dimensions
    const int16_t cardW = (SCREEN_WIDTH - 30) / 2;
    const int16_t cardH = 80;
    const int16_t margin = 10;

    // Row 1
    draw_card(margin, CONTENT_Y + 40, cardW, cardH, "Moisture", 0, COLOR_CYAN);
    draw_card(margin + cardW + 10, CONTENT_Y + 40, cardW, cardH, "Nitrogen", 0, COLOR_GREEN);

    // Row 2
    draw_card(margin, CONTENT_Y + 40 + cardH + 10, cardW, cardH, "Phosphorus", 0, COLOR_ORANGE);
    draw_card(margin + cardW + 10, CONTENT_Y + 40 + cardH + 10, cardW, cardH, "Potassium", 0, COLOR_MAGENTA);

    // Register values
    ui_registerValue(LABEL_MOISTURE, 20, CONTENT_Y + 75, 0);
    ui_registerValue(LABEL_NITROGEN, margin + cardW + 20, CONTENT_Y + 75, 0);
    ui_registerValue(LABEL_PHOSPHORUS, 20, CONTENT_Y + 75 + cardH + 10, 0);
    ui_registerValue(LABEL_POTASSIUM, margin + cardW + 20, CONTENT_Y + 75 + cardH + 10, 0);
}