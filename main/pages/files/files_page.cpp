#include "files_page.h"
#include "../../ui_engine.h"
#include "../../drawing.h"
#include "../../file_browser.h"
#include "../../simple_font.h"
#include <stdio.h>
#include <string.h>

// External reference to global SD browser
extern FileBrowser sdBrowser;

// Internal state for this page
static int lastTouchY = -1;
static int touchStartY = -1;
static bool firstFileDraw = true;

// --- Helper Functions (Static to avoid linking errors) ---

static void drawSimpleChar(int16_t x, int16_t y, char c, uint16_t fgColor, uint16_t bgColor) {
    if (c < 0x20 || c > 0x7E) c = '?';
    uint8_t charIndex = c - 0x20;
    for (int col = 0; col < FONT_WIDTH; col++) {
        uint8_t columnData = pgm_read_byte(&font5x7[charIndex][col]);
        for (int row = 0; row < FONT_HEIGHT; row++) {
            if (columnData & (1 << row)) draw_pixel(x + col, y + row, fgColor);
            else draw_pixel(x + col, y + row, bgColor);
        }
    }
    for (int row = 0; row < FONT_HEIGHT; row++) draw_pixel(x + FONT_WIDTH, y + row, bgColor);
}

static void drawTruncatedText(int16_t x, int16_t y, const char* text, int16_t maxWidth, uint16_t bgColor) {
    const int charWidth = FONT_WIDTH + 1;
    int len = strlen(text);
    int maxChars = maxWidth / charWidth;
    if (maxChars <= 0) return;
    const char* ext = strrchr(text, '.');
    int extLen = ext ? strlen(ext) : 0;
    int x_cursor = x;
    
    if (len <= maxChars) {
        for (int i = 0; i < len; i++) {
            drawSimpleChar(x_cursor, y, text[i], COLOR_BLACK, bgColor);
            x_cursor += charWidth;
        }
        int remainingWidth = maxWidth - (len * charWidth);
        if (remainingWidth > 0) draw_fillRect(x_cursor, y, remainingWidth, FONT_HEIGHT, bgColor);
    } else {
        int dotsNeeded = 3;
        int charsForName = maxChars - dotsNeeded - extLen;
        if (charsForName < 1) {
            for (int i = 0; i < maxChars - dotsNeeded && i < len; i++) {
                drawSimpleChar(x_cursor, y, text[i], COLOR_BLACK, bgColor);
                x_cursor += charWidth;
            }
            for (int i = 0; i < dotsNeeded; i++) {
                drawSimpleChar(x_cursor, y, '.', COLOR_BLACK, bgColor);
                x_cursor += charWidth;
            }
        } else {
            for (int i = 0; i < charsForName; i++) {
                drawSimpleChar(x_cursor, y, text[i], COLOR_BLACK, bgColor);
                x_cursor += charWidth;
            }
            for (int i = 0; i < dotsNeeded; i++) {
                drawSimpleChar(x_cursor, y, '.', COLOR_BLACK, bgColor);
                x_cursor += charWidth;
            }
            if (ext && extLen > 0) {
                for (int i = 0; i < extLen; i++) {
                    drawSimpleChar(x_cursor, y, ext[i], COLOR_BLACK, bgColor);
                    x_cursor += charWidth;
                }
            }
        }
        int usedWidth = x_cursor - x;
        int remainingWidth = maxWidth - usedWidth;
        if (remainingWidth > 0) draw_fillRect(x_cursor, y, remainingWidth, FONT_HEIGHT, bgColor);
    }
}

// --- Page Implementation ---

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
    
    bool scrollChanged = (currentScrollOffset != lastScrollOffset);
    bool fileCountChanged = (currentFileCount != lastFileCount);
    bool selectionChanged = (currentSelectedIndex != lastSelectedIndex);
    
    if (fullRedraw || fileCountChanged) {
        SerialUSB.println(F("Full redraw"));
        draw_fillRect(0, CONTENT_Y, SCREEN_WIDTH, CONTENT_HEIGHT, COLOR_WHITE);
        draw_fillRect(margin, CONTENT_Y + 5, itemWidth, 25, COLOR_BLUE);
        draw_fillRect(margin + 2, CONTENT_Y + 7, itemWidth - 4, 21, COLOR_DARKGRAY);
        SerialUSB.print(F("Current path: "));
        SerialUSB.println(sdBrowser.getCurrentPath());
        lastScrollOffset = -1; 
        lastSelectedIndex = -1;
    }
    
    int yPos = CONTENT_Y + 35;
    
    if (sdBrowser.canGoUp()) {
        if (fullRedraw || fileCountChanged) {
            draw_fillRect(margin, yPos, itemWidth, itemHeight, COLOR_GRAY);
            draw_rect(margin, yPos, itemWidth, itemHeight, COLOR_DARKGRAY);
            draw_fillRect(margin + 5, yPos + 7, iconSize, iconSize, COLOR_YELLOW);
            draw_fillRect(margin + 8, yPos + 10, iconSize - 6, iconSize - 6, COLOR_DARKGRAY);
            drawTruncatedText(margin + iconSize + 10, yPos + 15, "..", 120, COLOR_GRAY);
        }
        yPos += itemHeight + 5;
    }
    
    int visibleItems = 4;
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
        draw_fillRect(margin + 10, CONTENT_Y + 100, itemWidth - 20, 40, COLOR_RED);
        draw_fillRect(margin + 12, CONTENT_Y + 102, itemWidth - 24, 36, COLOR_WHITE);
        drawTruncatedText(margin + 30, CONTENT_Y + 115, "No Files Found", itemWidth - 60, COLOR_WHITE);
        lastScrollOffset = currentScrollOffset;
        lastFileCount = currentFileCount;
        lastSelectedIndex = currentSelectedIndex;
        return;
    }
    
    if (scrollChanged || selectionChanged || fullRedraw || fileCountChanged) {
        int itemsStartY = yPos;
        int itemsEndY = yPos + (visibleItems * (itemHeight + 5));
        
        if (!fullRedraw && !fileCountChanged) {
            draw_fillRect(margin, itemsStartY, itemWidth, itemsEndY - itemsStartY, COLOR_WHITE);
        }
        
        for (int i = 0; i < visibleItems && (currentScrollOffset + i) < totalFiles; i++) {
            FileEntry* entry = sdBrowser.getFile(currentScrollOffset + i);
            if (!entry) break;
            
            uint16_t bgColor = (currentScrollOffset + i == currentSelectedIndex) ? COLOR_CYAN : COLOR_LIGHTGRAY;
            draw_fillRect(margin, yPos, itemWidth, itemHeight, bgColor);
            draw_rect(margin, yPos, itemWidth, itemHeight, COLOR_DARKGRAY);
            
            uint16_t iconColor = entry->isDirectory ? COLOR_YELLOW : COLOR_BLUE;
            draw_fillRect(margin + 5, yPos + 7, iconSize, iconSize, iconColor);
            
            if (entry->isDirectory) {
                draw_fillRect(margin + 5, yPos + 7, 15, 8, COLOR_ORANGE);
            } else {
                draw_hLine(margin + 10, yPos + 12, 20, COLOR_WHITE);
                draw_hLine(margin + 10, yPos + 17, 20, COLOR_WHITE);
                draw_hLine(margin + 10, yPos + 22, 20, COLOR_WHITE);
                draw_hLine(margin + 10, yPos + 27, 20, COLOR_WHITE);
            }
            
            drawTruncatedText(margin + iconSize + 10, yPos + 15, entry->name, 120, bgColor);
            
            if (!entry->isDirectory) {
                draw_fillRect(margin + itemWidth - 60, yPos + 10, 55, 25, COLOR_WHITE);
                draw_rect(margin + itemWidth - 60, yPos + 10, 55, 25, COLOR_DARKGRAY);
                char sizeStr[16];
                if (entry->size < 1024) snprintf(sizeStr, sizeof(sizeStr), "%uB", (unsigned int)entry->size);
                else if (entry->size < 1024 * 1024) snprintf(sizeStr, sizeof(sizeStr), "%uK", (unsigned int)(entry->size / 1024));
                else snprintf(sizeStr, sizeof(sizeStr), "%uM", (unsigned int)(entry->size / (1024 * 1024)));
                drawTruncatedText(margin + itemWidth - 55, yPos + 18, sizeStr, 50, COLOR_WHITE);
            } else {
                draw_fillRect(margin + itemWidth - 45, yPos + 15, 40, 15, COLOR_ORANGE);
                drawTruncatedText(margin + itemWidth - 42, yPos + 18, "DIR", 35, COLOR_ORANGE);
            }
            yPos += itemHeight + 5;
        }
    }
    
    if (totalFiles > visibleItems) {
        int totalScrollable = totalFiles - visibleItems;
        // FIX: Removed std::max, just use max macro
        int scrollBarHeight = max(20, (CONTENT_HEIGHT - 40) * visibleItems / totalFiles);
        int scrollBarMaxY = CONTENT_HEIGHT - 40 - scrollBarHeight;
        int scrollBarY = CONTENT_Y + 35 + (totalScrollable > 0 ? (currentScrollOffset * scrollBarMaxY) / totalScrollable : 0);
        if (scrollChanged || fullRedraw || fileCountChanged) {
            draw_fillRect(SCREEN_WIDTH - 10, CONTENT_Y + 35, 8, CONTENT_HEIGHT - 40, COLOR_LIGHTGRAY);
            draw_fillRect(SCREEN_WIDTH - 10, scrollBarY, 8, scrollBarHeight, COLOR_BLUE);
        }
    }
    
    lastScrollOffset = currentScrollOffset;
    lastFileCount = currentFileCount;
    lastSelectedIndex = currentSelectedIndex;
    
    if (scrollChanged || selectionChanged || fullRedraw || fileCountChanged) {
        SerialUSB.println(F("=== Files Screen Update Complete ===\n"));
    }
}

void screen_files_handleTouch(int16_t x, int16_t y) {
    SerialUSB.print(F("Files touch: x=")); SerialUSB.print(x);
    SerialUSB.print(F(", y=")); SerialUSB.println(y);
    
    const int16_t itemHeight = 45;
    const int16_t margin = 5;
    
    if (touchStartY == -1) {
        touchStartY = y;
        lastTouchY = y;
        SerialUSB.println(F("Touch start"));
        return;
    }
    
    int dragDelta = lastTouchY - y;
    if (abs(dragDelta) > 10) {
        SerialUSB.print(F("Dragging, delta: ")); SerialUSB.println(dragDelta);
        sdBrowser.scroll(dragDelta > 0 ? 1 : -1);
        lastTouchY = y;
        screen_files_draw();
        return;
    }
    
    if (abs(y - touchStartY) < 15) {
        SerialUSB.println(F("Detected click"));
        int yPos = CONTENT_Y + 35;
        if (sdBrowser.canGoUp()) {
            if (y >= yPos && y < yPos + itemHeight) {
                SerialUSB.println(F("Up button clicked"));
                sdBrowser.goUp();
                firstFileDraw = true;
                ui_requestRedraw();
                touchStartY = -1; lastTouchY = -1;
                return;
            }
            yPos += itemHeight + 5;
        }
        int scrollOffset = sdBrowser.getScrollOffset();
        for (int i = 0; i < 4; i++) {
            if ((scrollOffset + i) >= sdBrowser.getFileCount()) break;
            if (y >= yPos && y < yPos + itemHeight) {
                SerialUSB.print(F("File item clicked: index ")); SerialUSB.println(scrollOffset + i);
                FileEntry* entry = sdBrowser.getFile(scrollOffset + i);
                if (entry && entry->isDirectory) firstFileDraw = true;
                sdBrowser.selectFile(scrollOffset + i);
                screen_files_draw();
                touchStartY = -1; lastTouchY = -1;
                return;
            }
            yPos += itemHeight + 5;
        }
    }
    SerialUSB.println(F("Touch released"));
    touchStartY = -1; lastTouchY = -1;
}