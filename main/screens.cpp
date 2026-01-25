/**
 * @file screens.cpp
 * @brief Screen Implementations
 */

#include "screens.h"
#include "ui_engine.h"
#include "drawing.h"
#include "file_browser.h"

// External reference to global SD browser
extern FileBrowser sdBrowser;

// File screen touch state
static int lastTouchY = -1;
static int touchStartY = -1;

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
    
    const int16_t itemHeight = 45;
    const int16_t margin = 5;
    const int16_t itemWidth = SCREEN_WIDTH - (margin * 2);
    const int16_t iconSize = 30;
    
    // Draw current path header
    draw_fillRect(margin, CONTENT_Y + 5, itemWidth, 25, COLOR_BLUE);
    draw_fillRect(margin + 2, CONTENT_Y + 7, itemWidth - 4, 21, COLOR_DARKGRAY);
    // Path text would be drawn here with font library
    // For now, just a visual indicator
    
    // Starting Y position for items
    int yPos = CONTENT_Y + 35;
    
    // Draw "Up" button if not in root directory
    if (sdBrowser.canGoUp()) {
        draw_fillRect(margin, yPos, itemWidth, itemHeight, COLOR_GRAY);
        draw_rect(margin, yPos, itemWidth, itemHeight, COLOR_DARKGRAY);
        
        // Up arrow icon (yellow folder with ..)
        draw_fillRect(margin + 5, yPos + 7, iconSize, iconSize, COLOR_YELLOW);
        draw_fillRect(margin + 8, yPos + 10, iconSize - 6, iconSize - 6, COLOR_DARKGRAY);
        
        yPos += itemHeight + 5;
    }
    
    // Calculate visible items
    int scrollOffset = sdBrowser.getScrollOffset();
    int visibleItems = 4; // Show 4 files at a time
    
    // Draw files and folders
    for (int i = 0; i < visibleItems && (scrollOffset + i) < sdBrowser.getFileCount(); i++) {
        FileEntry* entry = sdBrowser.getFile(scrollOffset + i);
        if (!entry) break;
        
        // Item background - highlight if selected
        uint16_t bgColor = COLOR_LIGHTGRAY;
        if (scrollOffset + i == sdBrowser.getSelectedIndex()) {
            bgColor = COLOR_CYAN;
        }
        
        draw_fillRect(margin, yPos, itemWidth, itemHeight, bgColor);
        draw_rect(margin, yPos, itemWidth, itemHeight, COLOR_DARKGRAY);
        
        // Icon - yellow for folders, blue for files
        uint16_t iconColor = entry->isDirectory ? COLOR_YELLOW : COLOR_BLUE;
        draw_fillRect(margin + 5, yPos + 7, iconSize, iconSize, iconColor);
        
        // Inner icon detail
        if (entry->isDirectory) {
            // Folder tab
            draw_fillRect(margin + 5, yPos + 7, 15, 8, COLOR_ORANGE);
        } else {
            // File lines
            draw_hLine(margin + 10, yPos + 12, 20, COLOR_WHITE);
            draw_hLine(margin + 10, yPos + 17, 20, COLOR_WHITE);
            draw_hLine(margin + 10, yPos + 22, 20, COLOR_WHITE);
            draw_hLine(margin + 10, yPos + 27, 20, COLOR_WHITE);
        }
        
        // File name area (text would be drawn here with font)
        // For now, just a placeholder
        draw_fillRect(margin + iconSize + 10, yPos + 5, 120, 15, bgColor);
        
        // File size (if not directory)
        if (!entry->isDirectory) {
            // Size display area
            draw_fillRect(margin + itemWidth - 60, yPos + 10, 55, 25, COLOR_WHITE);
            draw_rect(margin + itemWidth - 60, yPos + 10, 55, 25, COLOR_DARKGRAY);
            // Size text would be drawn here with font
        } else {
            // "[DIR]" indicator
            draw_fillRect(margin + itemWidth - 45, yPos + 15, 40, 15, COLOR_ORANGE);
        }
        
        yPos += itemHeight + 5;
    }
    
    // Draw scroll indicator if needed
    if (sdBrowser.getFileCount() > visibleItems) {
        int totalScrollable = sdBrowser.getFileCount() - visibleItems;
        int scrollBarHeight = max(20, (CONTENT_HEIGHT - 40) * visibleItems / sdBrowser.getFileCount());
        int scrollBarMaxY = CONTENT_HEIGHT - 40 - scrollBarHeight;
        int scrollBarY = CONTENT_Y + 35 + (totalScrollable > 0 ? 
            (scrollOffset * scrollBarMaxY) / totalScrollable : 0);
        
        // Scroll track
        draw_fillRect(SCREEN_WIDTH - 10, CONTENT_Y + 35, 8, CONTENT_HEIGHT - 40, COLOR_LIGHTGRAY);
        // Scroll thumb
        draw_fillRect(SCREEN_WIDTH - 10, scrollBarY, 8, scrollBarHeight, COLOR_BLUE);
    }
}

void screen_files_handleTouch(int16_t x, int16_t y) {
    const int16_t itemHeight = 45;
    const int16_t margin = 5;
    
    // Handle scrolling with drag
    if (touchStartY == -1) {
        touchStartY = y;
        lastTouchY = y;
        return;
    }
    
    int dragDelta = lastTouchY - y;
    if (abs(dragDelta) > 10) {
        // User is dragging - scroll
        sdBrowser.scroll(dragDelta > 0 ? 1 : -1);
        lastTouchY = y;
        ui_requestRedraw();
        return;
    }
    
    // Click detection (not dragging)
    if (abs(y - touchStartY) < 15) {
        // It's a click
        int yPos = CONTENT_Y + 35;
        
        // Check "Up" button
        if (sdBrowser.canGoUp()) {
            if (y >= yPos && y < yPos + itemHeight) {
                sdBrowser.goUp();
                ui_requestRedraw();
                touchStartY = -1;
                lastTouchY = -1;
                return;
            }
            yPos += itemHeight + 5;
        }
        
        // Check file/folder items
        int scrollOffset = sdBrowser.getScrollOffset();
        for (int i = 0; i < 4; i++) {
            if ((scrollOffset + i) >= sdBrowser.getFileCount()) break;
            
            if (y >= yPos && y < yPos + itemHeight) {
                sdBrowser.selectFile(scrollOffset + i);
                ui_requestRedraw();
                touchStartY = -1;
                lastTouchY = -1;
                return;
            }
            yPos += itemHeight + 5;
        }
    }
    
    // Reset touch state on release
    touchStartY = -1;
    lastTouchY = -1;
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