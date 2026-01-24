/**
 * @file screens.cpp
 * @brief Screen Implementations
 */

#include "screens.h"
#include "ui_engine.h"
#include "drawing.h"
#include <SD.h>

// ===================================
// Home Screen - Sensor Dashboard
// ===================================

void screen_home_draw(void)
{
    // Clear content area
    draw_fillRect(0, CONTENT_Y, SCREEN_WIDTH, CONTENT_HEIGHT, COLOR_WHITE);

    // Calculate card dimensions
    const int16_t cardW = (SCREEN_WIDTH - 30) / 2; // 105 pixels
    const int16_t cardH = 80;
    const int16_t margin = 10;

    // Row 1: Moisture and Nitrogen
    draw_card(margin, CONTENT_Y + 40, cardW, cardH,
              "Moisture", 0, COLOR_CYAN);
    draw_card(margin + cardW + 10, CONTENT_Y + 40, cardW, cardH,
              "Nitrogen", 0, COLOR_GREEN);

    // Row 2: Phosphorus and Potassium
    draw_card(margin, CONTENT_Y + 40 + cardH + 10, cardW, cardH,
              "Phosphorus", 0, COLOR_ORANGE);
    draw_card(margin + cardW + 10, CONTENT_Y + 40 + cardH + 10, cardW, cardH,
              "Potassium", 0, COLOR_MAGENTA);

    // Register values for dynamic updates
    ui_registerValue(LABEL_MOISTURE, 20, CONTENT_Y + 75, 0);
    ui_registerValue(LABEL_NITROGEN, margin + cardW + 20, CONTENT_Y + 75, 0);
    ui_registerValue(LABEL_PHOSPHORUS, 20, CONTENT_Y + 75 + cardH + 10, 0);
    ui_registerValue(LABEL_POTASSIUM, margin + cardW + 20, CONTENT_Y + 75 + cardH + 10, 0);
}

// ===================================
// Files Screen - SD Card Browser
// ===================================

void screen_files_draw(void)
{
    // Clear content area
    draw_fillRect(0, CONTENT_Y, SCREEN_WIDTH, CONTENT_HEIGHT, COLOR_WHITE);

    UIState *state = ui_getState();

    // Check if SD card is available
    if (!SD.begin(SD_CS))
    {
        // Display error message
        draw_fillRect(20, CONTENT_Y + 100, SCREEN_WIDTH - 40, 40, COLOR_RED);
        // Text would go here: "SD Card Error"
        return;
    }

    // Open root directory
    File root = SD.open("/");
    if (!root)
    {
        return;
    }

    // Count total files first
    state->totalFiles = 0;
    root.rewindDirectory();
    while (File entry = root.openNextFile())
    {
        if (!entry.isDirectory())
        {
            state->totalFiles++;
        }
        entry.close();
    }

    // Calculate visible items
    const int16_t visibleItems = (CONTENT_HEIGHT - 10) / FILE_ITEM_HEIGHT;
    const int16_t margin = 5;
    const int16_t itemWidth = SCREEN_WIDTH - (margin * 2) - SCROLLBAR_WIDTH - 5;

    // Reset directory to beginning
    root.rewindDirectory();

    // Skip files according to scroll offset
    int16_t currentIndex = 0;
    int16_t displayIndex = 0;

    while (File entry = root.openNextFile())
    {
        if (entry.isDirectory())
        {
            entry.close();
            continue;
        }

        // Skip files before scroll offset
        if (currentIndex < state->fileScrollOffset)
        {
            currentIndex++;
            entry.close();
            continue;
        }

        // Stop if we've displayed enough items
        if (displayIndex >= visibleItems)
        {
            entry.close();
            break;
        }

        // Calculate position
        int16_t y = CONTENT_Y + margin + (displayIndex * FILE_ITEM_HEIGHT);

        // Draw file item background
        draw_fillRect(margin, y, itemWidth, FILE_ITEM_HEIGHT - 2, COLOR_LIGHTGRAY);
        draw_rect(margin, y, itemWidth, FILE_ITEM_HEIGHT - 2, COLOR_DARKGRAY);

        // Draw file icon (simple square)
        draw_fillRect(margin + 5, y + 5, 25, 25, COLOR_BLUE);
        draw_rect(margin + 5, y + 5, 25, 25, COLOR_DARKGRAY);

        // File name would be drawn here (requires text rendering)
        // For now, just show the icon and box

        // Get file size for display
        uint32_t fileSize = entry.size();

        entry.close();
        displayIndex++;
        currentIndex++;
    }

    root.close();

    // Draw scrollbar on the right side
    if (state->totalFiles > visibleItems)
    {
        int16_t scrollbarX = SCREEN_WIDTH - SCROLLBAR_WIDTH - 2;
        int16_t scrollbarY = CONTENT_Y + margin;
        int16_t scrollbarHeight = CONTENT_HEIGHT - (margin * 2);

        // Scrollbar background
        draw_fillRect(scrollbarX, scrollbarY, SCROLLBAR_WIDTH, scrollbarHeight, COLOR_LIGHTGRAY);
        draw_rect(scrollbarX, scrollbarY, SCROLLBAR_WIDTH, scrollbarHeight, COLOR_DARKGRAY);

        // Calculate scrollbar thumb size and position
        float scrollRatio = (float)visibleItems / (float)state->totalFiles;
        int16_t thumbHeight = max(20, (int16_t)(scrollbarHeight * scrollRatio));

        float scrollPosition = (float)state->fileScrollOffset / (float)(state->totalFiles - visibleItems);
        int16_t thumbY = scrollbarY + (int16_t)((scrollbarHeight - thumbHeight) * scrollPosition);

        // Draw scrollbar thumb
        draw_fillRect(scrollbarX + 1, thumbY, SCROLLBAR_WIDTH - 2, thumbHeight, COLOR_BLUE);
    }
}

// ===================================
// AI Screen
// ===================================

void screen_ai_draw(void)
{
    // Clear content area
    draw_fillRect(0, CONTENT_Y, SCREEN_WIDTH, CONTENT_HEIGHT, COLOR_WHITE);

    const int16_t margin = 10;

    // AI status/recommendation area
    draw_fillRect(margin, CONTENT_Y + margin,
                  SCREEN_WIDTH - (margin * 2), 60, COLOR_CYAN);

    // AI response/chat area
    draw_fillRect(margin, CONTENT_Y + margin + 70,
                  SCREEN_WIDTH - (margin * 2), 100, COLOR_LIGHTGRAY);
}

// ===================================
// Settings Screen
// ===================================

// Button callback placeholders
static void onWifiClick(void)
{
    // Handle WiFi settings
}

static void onLanguageClick(void)
{
    // Handle language settings
}

static void onAboutClick(void)
{
    // Handle about dialog
}

void screen_settings_draw(void)
{
    // Clear content area
    draw_fillRect(0, CONTENT_Y, SCREEN_WIDTH, CONTENT_HEIGHT, COLOR_WHITE);

    const int16_t margin = 10;
    const int16_t buttonHeight = 40;
    const int16_t buttonWidth = SCREEN_WIDTH - (margin * 2);
    const int16_t spacing = 50;

    // Clear and add buttons
    ui_clearButtons();

    // WiFi button
    ui_addButton(margin, CONTENT_Y + 20, buttonWidth, buttonHeight,
                 "WiFi", COLOR_BLUE, onWifiClick);

    // Language button
    ui_addButton(margin, CONTENT_Y + 20 + spacing, buttonWidth, buttonHeight,
                 "Language", COLOR_GREEN, onLanguageClick);

    // About button
    ui_addButton(margin, CONTENT_Y + 20 + (spacing * 2), buttonWidth, buttonHeight,
                 "About", COLOR_ORANGE, onAboutClick);

    // Draw all buttons
    for (int i = 0; i < ui_getButtonCount(); i++)
    {
        UIButton *btn = ui_getButton(i);
        if (btn != NULL)
        {
            draw_button(btn);
        }
    }
}

// ===================================
// Input Screen - Numeric Keyboard
// ===================================

void screen_input_draw(void)
{
    // Clear content area
    draw_fillRect(0, CONTENT_Y, SCREEN_WIDTH, CONTENT_HEIGHT, COLOR_WHITE);

    const int16_t margin = 20;

    // Input field
    draw_fillRect(margin, CONTENT_Y + 20,
                  SCREEN_WIDTH - (margin * 2), 40, COLOR_LIGHTGRAY);
    draw_rect(margin, CONTENT_Y + 20,
              SCREEN_WIDTH - (margin * 2), 40, COLOR_DARKGRAY);

    // Numeric keyboard layout
    const int16_t keyW = 40;
    const int16_t keyH = 45;
    const int16_t keySpacing = 4;
    const int16_t keysPerRow = 5;
    const int16_t startX = 10;
    const int16_t startY = CONTENT_Y + 80;

    // Draw number keys 1-9 and 0
    for (int i = 0; i < 10; i++)
    {
        int row = i / keysPerRow;
        int col = i % keysPerRow;

        int16_t x = startX + col * (keyW + keySpacing);
        int16_t y = startY + row * (keyH + keySpacing);

        // Key background
        draw_fillRect(x, y, keyW, keyH, COLOR_BLUE);
        // Key face
        draw_fillRect(x + 2, y + 2, keyW - 4, keyH - 4, COLOR_WHITE);

        // Note: Number labels would require font rendering
    }

    // Additional keys (backspace, enter, etc.) could be added here
    int16_t funcY = startY + 2 * (keyH + keySpacing);

    // Backspace
    draw_fillRect(startX, funcY, keyW * 2 + keySpacing, keyH, COLOR_RED);
    draw_fillRect(startX + 2, funcY + 2, keyW * 2 + keySpacing - 4, keyH - 4, COLOR_WHITE);

    // Enter
    draw_fillRect(startX + (keyW + keySpacing) * 3, funcY, keyW * 2 + keySpacing, keyH, COLOR_GREEN);
    draw_fillRect(startX + (keyW + keySpacing) * 3 + 2, funcY + 2,
                  keyW * 2 + keySpacing - 4, keyH - 4, COLOR_WHITE);
}
