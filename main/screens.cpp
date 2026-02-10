/**
 * @file screens.cpp
 * @brief Screen Implementations (AI, Settings, Debug)
 */

#include "screens.h"
#include "ui_engine.h"
#include "drawing.h"
#include "file_browser.h"
#include "simple_font.h"
#include <stdio.h>
#include <string.h> // Added for strlen/strrchr
#include "a9g_gps.h"

// External reference to global SD browser
extern FileBrowser sdBrowser;

// External reference to GPS module
extern A9G_GPS gpsModule;

// ===================================
// Text Rendering Helpers
// ===================================

// Simple 5x7 character renderer using bitmap font
void drawSimpleChar(int16_t x, int16_t y, char c, uint16_t fgColor, uint16_t bgColor) {
    if (c < 0x20 || c > 0x7E) c = '?';
    
    uint8_t charIndex = c - 0x20;
    
    for (int col = 0; col < FONT_WIDTH; col++) {
        uint8_t columnData = pgm_read_byte(&font5x7[charIndex][col]);
        for (int row = 0; row < FONT_HEIGHT; row++) {
            if (columnData & (1 << row)) {
                draw_pixel(x + col, y + row, fgColor);
            } else {
                draw_pixel(x + col, y + row, bgColor);
            }
        }
    }
    
    // Spacing
    for (int row = 0; row < FONT_HEIGHT; row++) {
        draw_pixel(x + FONT_WIDTH, y + row, bgColor);
    }
}

// Helper function to draw text with truncation
void drawTruncatedText(int16_t x, int16_t y, const char* text, int16_t maxWidth, uint16_t bgColor) {
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
        if (remainingWidth > 0) {
            draw_fillRect(x_cursor, y, remainingWidth, FONT_HEIGHT, bgColor);
        }
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
        if (remainingWidth > 0) {
            draw_fillRect(x_cursor, y, remainingWidth, FONT_HEIGHT, bgColor);
        }
    }
}

// Helper to draw GPS text in header
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
// AI Screen
// ===================================

void screen_ai_draw(void)
{
    draw_fillRect(0, CONTENT_Y, SCREEN_WIDTH, CONTENT_HEIGHT, COLOR_WHITE);
    const int16_t margin = 10;
    draw_fillRect(margin, CONTENT_Y + margin, SCREEN_WIDTH - (margin * 2), 60, COLOR_CYAN);
    draw_fillRect(margin, CONTENT_Y + margin + 70, SCREEN_WIDTH - (margin * 2), 100, COLOR_LIGHTGRAY);
}

// ===================================
// Settings Screen
// ===================================

static void onWifiClick(void) { SerialUSB.println(F("WiFi button clicked")); }
static void onLanguageClick(void) { SerialUSB.println(F("Language button clicked")); }
static void onAboutClick(void) { SerialUSB.println(F("About button clicked")); }

void screen_settings_draw(void)
{
    draw_fillRect(0, CONTENT_Y, SCREEN_WIDTH, CONTENT_HEIGHT, COLOR_WHITE);
    const int16_t margin = 10;
    const int16_t buttonHeight = 40;
    const int16_t buttonWidth = SCREEN_WIDTH - (margin * 2);
    const int16_t spacing = 50;

    ui_clearButtons();
    ui_addButton(margin, CONTENT_Y + 20, buttonWidth, buttonHeight, "WiFi", COLOR_BLUE, onWifiClick);
    ui_addButton(margin, CONTENT_Y + 20 + spacing, buttonWidth, buttonHeight, "Language", COLOR_GREEN, onLanguageClick);
    ui_addButton(margin, CONTENT_Y + 20 + (spacing * 2), buttonWidth, buttonHeight, "About", COLOR_ORANGE, onAboutClick);

    for (int i = 0; i < ui_getButtonCount(); i++) {
        UIButton *btn = ui_getButton(i);
        if (btn != NULL) draw_button(btn);
    }
}

// ===================================
// Input Screen
// ===================================

void screen_input_draw(void)
{
    draw_fillRect(0, CONTENT_Y, SCREEN_WIDTH, CONTENT_HEIGHT, COLOR_WHITE);
    const int16_t margin = 20;
    draw_fillRect(margin, CONTENT_Y + 20, SCREEN_WIDTH - (margin * 2), 40, COLOR_LIGHTGRAY);
    draw_rect(margin, CONTENT_Y + 20, SCREEN_WIDTH - (margin * 2), 40, COLOR_DARKGRAY);

    const int16_t keyW = 40;
    const int16_t keyH = 45;
    const int16_t keySpacing = 4;
    const int16_t keysPerRow = 5;
    const int16_t startX = 10;
    const int16_t startY = CONTENT_Y + 80;

    for (int i = 0; i < 10; i++) {
        int row = i / keysPerRow;
        int col = i % keysPerRow;
        int16_t x = startX + col * (keyW + keySpacing);
        int16_t y = startY + row * (keyH + keySpacing);
        draw_fillRect(x, y, keyW, keyH, COLOR_BLUE);
        draw_fillRect(x + 2, y + 2, keyW - 4, keyH - 4, COLOR_WHITE);
    }

    int16_t funcY = startY + 2 * (keyH + keySpacing);
    draw_fillRect(startX, funcY, keyW * 2 + keySpacing, keyH, COLOR_RED);
    draw_fillRect(startX + 2, funcY + 2, keyW * 2 + keySpacing - 4, keyH - 4, COLOR_WHITE);
    draw_fillRect(startX + (keyW + keySpacing) * 3, funcY, keyW * 2 + keySpacing, keyH, COLOR_GREEN);
    draw_fillRect(startX + (keyW + keySpacing) * 3 + 2, funcY + 2, keyW * 2 + keySpacing - 4, keyH - 4, COLOR_WHITE);
}

// ===================================
// GPS Debug Screen
// ===================================

void screen_gps_debug_draw(void)
{
    SerialUSB.println(F("\n=== GPS Debug Screen Draw ==="));
    draw_fillRect(0, CONTENT_Y, SCREEN_WIDTH, CONTENT_HEIGHT, COLOR_BLACK);
    
    GPSData gpsData = gpsModule.getGPSData();
    GPSDebugInfo debugInfo = gpsModule.getDebugInfo();
    
    const int16_t margin = 5;
    int16_t yPos = CONTENT_Y + 5;
    
    draw_fillRect(margin, yPos, SCREEN_WIDTH - 2*margin, 20, COLOR_BLUE);
    drawTruncatedText(margin + 5, yPos + 6, "GPS DEBUG INFO", SCREEN_WIDTH - 2*margin - 10, COLOR_BLUE);
    yPos += 25;
    
    // Status
    draw_fillRect(margin, yPos, SCREEN_WIDTH - 2*margin, 15, COLOR_DARKGRAY);
    drawTruncatedText(margin + 3, yPos + 4, "Status:", SCREEN_WIDTH - 2*margin - 6, COLOR_DARKGRAY);
    yPos += 17;
    
    char statusText[64];
    if (gpsData.valid) snprintf(statusText, sizeof(statusText), "VALID FIX");
    else snprintf(statusText, sizeof(statusText), "NO FIX");
    
    draw_fillRect(margin, yPos, SCREEN_WIDTH - 2*margin, 12, gpsData.valid ? COLOR_GREEN : COLOR_RED);
    drawTruncatedText(margin + 3, yPos + 2, statusText, SCREEN_WIDTH - 2*margin - 6, gpsData.valid ? COLOR_GREEN : COLOR_RED);
    yPos += 14;
    
    // Coordinates
    draw_fillRect(margin, yPos, SCREEN_WIDTH - 2*margin, 15, COLOR_DARKGRAY);
    drawTruncatedText(margin + 3, yPos + 4, "Coordinates:", SCREEN_WIDTH - 2*margin - 6, COLOR_DARKGRAY);
    yPos += 17;
    
    char latText[32];
    snprintf(latText, sizeof(latText), "Lat: %.6f", gpsData.latitude);
    draw_fillRect(margin, yPos, SCREEN_WIDTH - 2*margin, 12, COLOR_LIGHTGRAY);
    drawTruncatedText(margin + 3, yPos + 2, latText, SCREEN_WIDTH - 2*margin - 6, COLOR_LIGHTGRAY);
    yPos += 14;
    
    char lonText[32];
    snprintf(lonText, sizeof(lonText), "Lon: %.6f", gpsData.longitude);
    draw_fillRect(margin, yPos, SCREEN_WIDTH - 2*margin, 12, COLOR_LIGHTGRAY);
    drawTruncatedText(margin + 3, yPos + 2, lonText, SCREEN_WIDTH - 2*margin - 6, COLOR_LIGHTGRAY);
    yPos += 14;
    
    // Commands
    yPos += 3;
    draw_fillRect(margin, yPos, SCREEN_WIDTH - 2*margin, 15, COLOR_DARKGRAY);
    drawTruncatedText(margin + 3, yPos + 4, "Last AT Command:", SCREEN_WIDTH - 2*margin - 6, COLOR_DARKGRAY);
    yPos += 17;
    draw_fillRect(margin, yPos, SCREEN_WIDTH - 2*margin, 12, COLOR_YELLOW);
    drawTruncatedText(margin + 3, yPos + 2, debugInfo.lastCommand, SCREEN_WIDTH - 2*margin - 6, COLOR_YELLOW);
    yPos += 14;
    
    // Buttons
    yPos = NAVBAR_Y - 45;
    draw_fillRect(10, yPos, 70, 35, COLOR_GREEN);
    draw_rect(10, yPos, 70, 35, COLOR_DARKGREEN);
    drawTruncatedText(20, yPos + 13, "REFRESH", 50, COLOR_GREEN);
    
    draw_fillRect(90, yPos, 70, 35, COLOR_BLUE);
    draw_rect(90, yPos, 70, 35, COLOR_DARKGRAY);
    drawTruncatedText(105, yPos + 13, "BACK", 50, COLOR_BLUE);
    
    draw_fillRect(170, yPos, 60, 35, COLOR_RED);
    draw_rect(170, yPos, 60, 35, COLOR_DARKGRAY);
    drawTruncatedText(178, yPos + 13, "CLEAR", 44, COLOR_RED);
    
    SerialUSB.println(F("=== GPS Debug Screen Complete ===\n"));
}

void screen_gps_debug_handleTouch(int16_t x, int16_t y)
{
    int16_t buttonY = NAVBAR_Y - 45;
    if (x >= 10 && x <= 80 && y >= buttonY && y <= buttonY + 35) {
        SerialUSB.println(F("GPS Debug: REFRESH clicked"));
        gpsModule.refreshDebugInfo();
        ui_requestRedraw();
        return;
    }
    if (x >= 90 && x <= 160 && y >= buttonY && y <= buttonY + 35) {
        SerialUSB.println(F("GPS Debug: BACK clicked"));
        ui_goBack();
        return;
    }
    if (x >= 170 && x <= 230 && y >= buttonY && y <= buttonY + 35) {
        SerialUSB.println(F("GPS Debug: CLEAR clicked - restarting GPS"));
        gpsModule.turnOffGPS();
        delay(1000);
        gpsModule.turnOnGPS();
        ui_requestRedraw();
        return;
    }
}

void screen_gps_debug_update(void)
{
    static unsigned long lastRefresh = 0;
    if (millis() - lastRefresh > 3000) {
        if (ui_getCurrentScreen() == SCREEN_GPS_DEBUG) {
            ui_requestRedraw();
        }
        lastRefresh = millis();
    }
}