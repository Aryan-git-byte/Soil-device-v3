/**
 * @file screens.cpp
 * @brief Screen Implementations with Debugging
 */

#include "screens.h"
#include "ui_engine.h"
#include "drawing.h"
#include "file_browser.h"
#include "simple_font.h"
#include <stdio.h>

// External reference to global SD browser
extern FileBrowser sdBrowser;

// File screen touch state
static int lastTouchY = -1;
static int touchStartY = -1;
static bool firstFileDraw = true;

// Forward declarations for text rendering
void drawSimpleChar(int16_t x, int16_t y, char c, uint16_t fgColor, uint16_t bgColor);
void drawTruncatedText(int16_t x, int16_t y, const char* text, int16_t maxWidth, uint16_t bgColor);
void drawGPSText(const char* text, int16_t x, int16_t y);

// ===================================
// Text Rendering Functions
// ===================================

// Simple 5x7 character renderer using bitmap font
void drawSimpleChar(int16_t x, int16_t y, char c, uint16_t fgColor, uint16_t bgColor) {
    // Check if character is in printable range
    if (c < 0x20 || c > 0x7E) {
        c = '?'; // Replace unprintable chars with ?
    }
    
    // Get font data from PROGMEM
    uint8_t charIndex = c - 0x20;
    
    // Draw each column of the character
    for (int col = 0; col < FONT_WIDTH; col++) {
        uint8_t columnData = pgm_read_byte(&font5x7[charIndex][col]);
        
        // Draw each pixel in the column
        for (int row = 0; row < FONT_HEIGHT; row++) {
            if (columnData & (1 << row)) {
                draw_pixel(x + col, y + row, fgColor);
            } else {
                draw_pixel(x + col, y + row, bgColor);
            }
        }
    }
    
    // Add 1 pixel spacing between characters
    for (int row = 0; row < FONT_HEIGHT; row++) {
        draw_pixel(x + FONT_WIDTH, y + row, bgColor);
    }
}

// Helper function to draw text with truncation
void drawTruncatedText(int16_t x, int16_t y, const char* text, int16_t maxWidth, uint16_t bgColor) {
    // Character width includes 1 pixel spacing
    const int charWidth = FONT_WIDTH + 1;
    
    int len = strlen(text);
    int maxChars = maxWidth / charWidth;
    
    if (maxChars <= 0) return;
    
    // Find extension if it's a file
    const char* ext = strrchr(text, '.');
    int extLen = ext ? strlen(ext) : 0;
    
    int x_cursor = x;
    
    if (len <= maxChars) {
        // Text fits, draw it all
        for (int i = 0; i < len; i++) {
            drawSimpleChar(x_cursor, y, text[i], COLOR_BLACK, bgColor);
            x_cursor += charWidth;
        }
        
        // Fill remaining space with background
        int remainingWidth = maxWidth - (len * charWidth);
        if (remainingWidth > 0) {
            draw_fillRect(x_cursor, y, remainingWidth, FONT_HEIGHT, bgColor);
        }
    } else {
        // Text too long, truncate with "..."
        int dotsNeeded = 3;
        int charsForName = maxChars - dotsNeeded - extLen;
        
        if (charsForName < 1) {
            // Not enough room for name + dots + extension, just show truncated name
            for (int i = 0; i < maxChars - dotsNeeded && i < len; i++) {
                drawSimpleChar(x_cursor, y, text[i], COLOR_BLACK, bgColor);
                x_cursor += charWidth;
            }
            for (int i = 0; i < dotsNeeded; i++) {
                drawSimpleChar(x_cursor, y, '.', COLOR_BLACK, bgColor);
                x_cursor += charWidth;
            }
        } else {
            // Draw first part of name
            for (int i = 0; i < charsForName; i++) {
                drawSimpleChar(x_cursor, y, text[i], COLOR_BLACK, bgColor);
                x_cursor += charWidth;
            }
            
            // Draw "..."
            for (int i = 0; i < dotsNeeded; i++) {
                drawSimpleChar(x_cursor, y, '.', COLOR_BLACK, bgColor);
                x_cursor += charWidth;
            }
            
            // Draw extension
            if (ext && extLen > 0) {
                for (int i = 0; i < extLen; i++) {
                    drawSimpleChar(x_cursor, y, ext[i], COLOR_BLACK, bgColor);
                    x_cursor += charWidth;
                }
            }
        }
        
        // Fill any remaining space with background
        int usedWidth = x_cursor - x;
        int remainingWidth = maxWidth - usedWidth;
        if (remainingWidth > 0) {
            draw_fillRect(x_cursor, y, remainingWidth, FONT_HEIGHT, bgColor);
        }
    }
}

// Helper function to draw GPS text in header (white on blue)
void drawGPSText(const char* text, int16_t x, int16_t y) {
    const int charWidth = FONT_WIDTH + 1;
    int len = strlen(text);
    
    int x_cursor = x;
    for (int i = 0; i < len; i++) {
        drawSimpleChar(x_cursor, y, text[i], COLOR_WHITE, COLOR_BLUE);
        x_cursor += charWidth;
    }
}

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
    static int lastScrollOffset = -1;
    static int lastFileCount = -1;
    static int lastSelectedIndex = -1;
    bool fullRedraw = false;
    
    if (firstFileDraw) {
        SerialUSB.println(F("\n=== Files Screen Draw ==="));
        fullRedraw = true;
        firstFileDraw = false;
    }
    
    const int16_t itemHeight = 45;
    const int16_t margin = 5;
    const int16_t itemWidth = SCREEN_WIDTH - (margin * 2);
    const int16_t iconSize = 30;
    
    int currentScrollOffset = sdBrowser.getScrollOffset();
    int currentFileCount = sdBrowser.getFileCount();
    int currentSelectedIndex = sdBrowser.getSelectedIndex();
    
    // Check what changed
    bool scrollChanged = (currentScrollOffset != lastScrollOffset);
    bool fileCountChanged = (currentFileCount != lastFileCount);
    bool selectionChanged = (currentSelectedIndex != lastSelectedIndex);
    
    // Full redraw only on first draw or file count change
    if (fullRedraw || fileCountChanged) {
        SerialUSB.println(F("Full redraw"));
        // Clear content area
        draw_fillRect(0, CONTENT_Y, SCREEN_WIDTH, CONTENT_HEIGHT, COLOR_WHITE);
        
        // Draw current path header
        draw_fillRect(margin, CONTENT_Y + 5, itemWidth, 25, COLOR_BLUE);
        draw_fillRect(margin + 2, CONTENT_Y + 7, itemWidth - 4, 21, COLOR_DARKGRAY);
        
        SerialUSB.print(F("Current path: "));
        SerialUSB.println(sdBrowser.getCurrentPath());
        
        lastScrollOffset = -1; // Force redraw of items
        lastSelectedIndex = -1;
    }
    
    // Starting Y position for items
    int yPos = CONTENT_Y + 35;
    
    // Draw "Up" button if not in root directory
    if (sdBrowser.canGoUp()) {
        if (fullRedraw || fileCountChanged) {
            SerialUSB.println(F("Drawing 'Up' button"));
            draw_fillRect(margin, yPos, itemWidth, itemHeight, COLOR_GRAY);
            draw_rect(margin, yPos, itemWidth, itemHeight, COLOR_DARKGRAY);
            
            // Up arrow icon (yellow folder with ..)
            draw_fillRect(margin + 5, yPos + 7, iconSize, iconSize, COLOR_YELLOW);
            draw_fillRect(margin + 8, yPos + 10, iconSize - 6, iconSize - 6, COLOR_DARKGRAY);
            
            // Draw ".." text
            drawTruncatedText(margin + iconSize + 10, yPos + 15, "..", 120, COLOR_GRAY);
        }
        yPos += itemHeight + 5;
    }
    
    // Calculate visible items
    int visibleItems = 4; // Show 4 files at a time
    int totalFiles = currentFileCount;
    
    if (scrollChanged || selectionChanged || fullRedraw || fileCountChanged) {
        SerialUSB.print(F("Redrawing items - Total files: "));
        SerialUSB.print(totalFiles);
        SerialUSB.print(F(", Scroll offset: "));
        SerialUSB.print(currentScrollOffset);
        SerialUSB.print(F(", Selected: "));
        SerialUSB.println(currentSelectedIndex);
    }
    
    if (totalFiles == 0) {
        SerialUSB.println(F("WARNING: No files to display!"));
        // Draw "No files" message
        draw_fillRect(margin + 10, CONTENT_Y + 100, itemWidth - 20, 40, COLOR_RED);
        draw_fillRect(margin + 12, CONTENT_Y + 102, itemWidth - 24, 36, COLOR_WHITE);
        drawTruncatedText(margin + 30, CONTENT_Y + 115, "No Files Found", itemWidth - 60, COLOR_WHITE);
        
        lastScrollOffset = currentScrollOffset;
        lastFileCount = currentFileCount;
        lastSelectedIndex = currentSelectedIndex;
        return;
    }
    
    // Only redraw items if scroll changed, selection changed, or full redraw needed
    if (scrollChanged || selectionChanged || fullRedraw || fileCountChanged) {
        // Calculate the content area for items
        int itemsStartY = yPos;
        int itemsEndY = yPos + (visibleItems * (itemHeight + 5));
        
        // Clear only the items area, not the whole screen
        if (!fullRedraw && !fileCountChanged) {
            draw_fillRect(margin, itemsStartY, itemWidth, itemsEndY - itemsStartY, COLOR_WHITE);
        }
        
        // Draw files and folders
        for (int i = 0; i < visibleItems && (currentScrollOffset + i) < totalFiles; i++) {
            FileEntry* entry = sdBrowser.getFile(currentScrollOffset + i);
            if (!entry) {
                SerialUSB.print(F("ERROR: getFile returned NULL for index "));
                SerialUSB.println(currentScrollOffset + i);
                break;
            }
            
            // Item background - highlight if selected
            uint16_t bgColor = COLOR_LIGHTGRAY;
            if (currentScrollOffset + i == currentSelectedIndex) {
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
            
            // Draw file name with truncation
            drawTruncatedText(margin + iconSize + 10, yPos + 15, entry->name, 120, bgColor);
            
            // File size (if not directory)
            if (!entry->isDirectory) {
                // Size display area
                draw_fillRect(margin + itemWidth - 60, yPos + 10, 55, 25, COLOR_WHITE);
                draw_rect(margin + itemWidth - 60, yPos + 10, 55, 25, COLOR_DARKGRAY);
                
                // Convert size to string
                char sizeStr[16];
                if (entry->size < 1024) {
                    snprintf(sizeStr, sizeof(sizeStr), "%uB", (unsigned int)entry->size);
                } else if (entry->size < 1024 * 1024) {
                    snprintf(sizeStr, sizeof(sizeStr), "%uK", (unsigned int)(entry->size / 1024));
                } else {
                    snprintf(sizeStr, sizeof(sizeStr), "%uM", (unsigned int)(entry->size / (1024 * 1024)));
                }
                drawTruncatedText(margin + itemWidth - 55, yPos + 18, sizeStr, 50, COLOR_WHITE);
            } else {
                // "[DIR]" indicator
                draw_fillRect(margin + itemWidth - 45, yPos + 15, 40, 15, COLOR_ORANGE);
                drawTruncatedText(margin + itemWidth - 42, yPos + 18, "DIR", 35, COLOR_ORANGE);
            }
            
            yPos += itemHeight + 5;
        }
    }
    
    // Draw scroll indicator if needed
    if (totalFiles > visibleItems) {
        int totalScrollable = totalFiles - visibleItems;
        int scrollBarHeight = max(20, (CONTENT_HEIGHT - 40) * visibleItems / totalFiles);
        int scrollBarMaxY = CONTENT_HEIGHT - 40 - scrollBarHeight;
        int scrollBarY = CONTENT_Y + 35 + (totalScrollable > 0 ? 
            (currentScrollOffset * scrollBarMaxY) / totalScrollable : 0);
        
        // Only redraw scrollbar if scroll changed or full redraw
        if (scrollChanged || fullRedraw || fileCountChanged) {
            // Scroll track
            draw_fillRect(SCREEN_WIDTH - 10, CONTENT_Y + 35, 8, CONTENT_HEIGHT - 40, COLOR_LIGHTGRAY);
            // Scroll thumb
            draw_fillRect(SCREEN_WIDTH - 10, scrollBarY, 8, scrollBarHeight, COLOR_BLUE);
        }
    }
    
    // Update state tracking
    lastScrollOffset = currentScrollOffset;
    lastFileCount = currentFileCount;
    lastSelectedIndex = currentSelectedIndex;
    
    if (scrollChanged || selectionChanged || fullRedraw || fileCountChanged) {
        SerialUSB.println(F("=== Files Screen Update Complete ===\n"));
    }
}

void screen_files_handleTouch(int16_t x, int16_t y) {
    SerialUSB.print(F("Files touch: x="));
    SerialUSB.print(x);
    SerialUSB.print(F(", y="));
    SerialUSB.println(y);
    
    const int16_t itemHeight = 45;
    const int16_t margin = 5;
    
    // Handle scrolling with drag
    if (touchStartY == -1) {
        touchStartY = y;
        lastTouchY = y;
        SerialUSB.println(F("Touch start"));
        return;
    }
    
    int dragDelta = lastTouchY - y;
    if (abs(dragDelta) > 10) {
        // User is dragging - scroll
        SerialUSB.print(F("Dragging, delta: "));
        SerialUSB.println(dragDelta);
        sdBrowser.scroll(dragDelta > 0 ? 1 : -1);
        lastTouchY = y;
        // Don't call ui_requestRedraw() - screen_files_draw() will detect scroll change
        // Just trigger the draw directly without full screen redraw
        screen_files_draw();
        return;
    }
    
    // Click detection (not dragging)
    if (abs(y - touchStartY) < 15) {
        SerialUSB.println(F("Detected click"));
        // It's a click
        int yPos = CONTENT_Y + 35;
        
        // Check "Up" button
        if (sdBrowser.canGoUp()) {
            if (y >= yPos && y < yPos + itemHeight) {
                SerialUSB.println(F("Up button clicked"));
                sdBrowser.goUp();
                firstFileDraw = true; // Force full redraw for new directory
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
                SerialUSB.print(F("File item clicked: index "));
                SerialUSB.println(scrollOffset + i);
                
                FileEntry* entry = sdBrowser.getFile(scrollOffset + i);
                if (entry && entry->isDirectory) {
                    // Entering directory - need full redraw
                    firstFileDraw = true;
                }
                
                sdBrowser.selectFile(scrollOffset + i);
                // Just redraw the file screen, not the whole UI
                screen_files_draw();
                touchStartY = -1;
                lastTouchY = -1;
                return;
            }
            yPos += itemHeight + 5;
        }
    }
    
    // Reset touch state on release
    SerialUSB.println(F("Touch released"));
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
    SerialUSB.println(F("WiFi button clicked"));
}

static void onLanguageClick(void)
{
    // Handle language settings
    SerialUSB.println(F("Language button clicked"));
}

static void onAboutClick(void)
{
    // Handle about dialog
    SerialUSB.println(F("About button clicked"));
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
    }

    // Additional keys (backspace, enter, etc.)
    int16_t funcY = startY + 2 * (keyH + keySpacing);

    // Backspace
    draw_fillRect(startX, funcY, keyW * 2 + keySpacing, keyH, COLOR_RED);
    draw_fillRect(startX + 2, funcY + 2, keyW * 2 + keySpacing - 4, keyH - 4, COLOR_WHITE);

    // Enter
    draw_fillRect(startX + (keyW + keySpacing) * 3, funcY, keyW * 2 + keySpacing, keyH, COLOR_GREEN);
    draw_fillRect(startX + (keyW + keySpacing) * 3 + 2, funcY + 2,
                  keyW * 2 + keySpacing - 4, keyH - 4, COLOR_WHITE);
}