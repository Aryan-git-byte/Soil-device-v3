/**
 * @file ui_engine.cpp
 * @brief UI Engine Implementation
 */

#include "ui_engine.h"
#include "drawing.h"
#include "touch_driver.h"
#include "screens.h"
#include "icons.h"
#include <string.h>

// ===================================
// Global State
// ===================================

static UIState uiState = {
    SCREEN_HOME,   // currentScreen
    SCREEN_HOME,   // lastScreen
    UI_ALERT_NONE, // alertType
    "",            // alertMsg
    0,             // alertTime
    true,          // needsFullRedraw
    true,          // needsNavbarRedraw
    -1,            // lastTouchX
    -1,            // lastTouchY
    0,             // lastTouchTime
    0,             // gsmSignal
    0,             // batteryLevel
    false,         // gpsLock
    0.0,           // gpsLatitude
    0.0,           // gpsLongitude
    false          // gpsValid
};

// Button storage
static UIButton buttons[MAX_BUTTONS];
static int buttonCount = 0;

// Value storage
static UIValue dataValues[MAX_VALUES];
static int valueCount = 0;

// Language strings
const char *labels_en[LABEL_COUNT] = {
    "Moisture", "Nitrogen", "Phosphorus", "Potassium",
    "Temperature", "Humidity", "pH"};

// ===================================
// Global State Access
// ===================================

UIState *ui_getState(void)
{
    return &uiState;
}

// ===================================
// UI Engine Initialization
// ===================================

void ui_init(void)
{
    uiState.currentScreen = SCREEN_HOME;
    uiState.lastScreen = SCREEN_HOME;
    uiState.alertType = UI_ALERT_NONE;
    uiState.alertMsg[0] = '\0';
    uiState.alertTime = 0;
    uiState.needsFullRedraw = true;
    uiState.needsNavbarRedraw = true;
    uiState.lastTouchX = -1;
    uiState.lastTouchY = -1;
    uiState.lastTouchTime = 0;

    buttonCount = 0;
    valueCount = 0;
}

// ===================================
// Button System
// ===================================

int ui_addButton(int16_t x, int16_t y, int16_t w, int16_t h,
                 const char *label, uint16_t color, void (*callback)(void))
{
    if (buttonCount >= MAX_BUTTONS)
    {
        return -1;
    }

    buttons[buttonCount].x = x;
    buttons[buttonCount].y = y;
    buttons[buttonCount].w = w;
    buttons[buttonCount].h = h;
    buttons[buttonCount].label = label;
    buttons[buttonCount].color = color;
    buttons[buttonCount].callback = callback;
    buttons[buttonCount].visible = true;

    return buttonCount++;
}

void ui_clearButtons(void)
{
    buttonCount = 0;
}

bool ui_checkButton(UIButton *btn, int16_t x, int16_t y)
{
    if (btn == NULL || !btn->visible)
    {
        return false;
    }
    return (x >= btn->x && x < btn->x + btn->w &&
            y >= btn->y && y < btn->y + btn->h);
}

UIButton *ui_getButton(int index)
{
    if (index < 0 || index >= buttonCount)
    {
        return NULL;
    }
    return &buttons[index];
}

int ui_getButtonCount(void)
{
    return buttonCount;
}

// ===================================
// Data Binding System
// ===================================

void ui_registerValue(LabelID id, int16_t x, int16_t y, int16_t initialValue)
{
    if (valueCount >= MAX_VALUES)
    {
        return;
    }

    dataValues[valueCount].id = id;
    dataValues[valueCount].x = x;
    dataValues[valueCount].y = y;
    dataValues[valueCount].value = initialValue;
    dataValues[valueCount].lastValue = initialValue;
    dataValues[valueCount].needsRedraw = true;

    valueCount++;
}

void ui_updateValue(LabelID id, int16_t newValue)
{
    for (int i = 0; i < valueCount; i++)
    {
        if (dataValues[i].id == id && dataValues[i].value != newValue)
        {
            dataValues[i].lastValue = dataValues[i].value;
            dataValues[i].value = newValue;
            dataValues[i].needsRedraw = true;
        }
    }
}

void ui_redrawValues(void)
{
    for (int i = 0; i < valueCount; i++)
    {
        if (dataValues[i].needsRedraw)
        {
            // Clear and redraw value area
            draw_fillRect(dataValues[i].x, dataValues[i].y, 80, 25, COLOR_LIGHTGRAY);
            dataValues[i].needsRedraw = false;
            // Note: Actual number rendering would require a font library
        }
    }
}

void ui_clearValues(void)
{
    valueCount = 0;
}

// ===================================
// Alert System
// ===================================

void ui_drawAlert(const char *text, AlertType type)
{
    if (type == UI_ALERT_NONE)
    {
        return;
    }

    uint16_t color;
    switch (type)
    {
    case UI_ALERT_ERROR:
        color = COLOR_RED;
        break;
    case UI_ALERT_WARN:
        color = COLOR_YELLOW;
        break;
    default:
        color = COLOR_CYAN;
        break;
    }

    draw_fillRect(0, CONTENT_Y, SCREEN_WIDTH, 30, color);
    draw_fillRect(2, CONTENT_Y + 2, SCREEN_WIDTH - 4, 26, COLOR_BLACK);
    // Note: Text rendering would require a font library
}

void ui_showAlert(const char *msg, AlertType type)
{
    strncpy(uiState.alertMsg, msg, MAX_ALERT_LEN - 1);
    uiState.alertMsg[MAX_ALERT_LEN - 1] = '\0';
    uiState.alertType = type;
    uiState.alertTime = millis();
    ui_drawAlert(msg, type);
}

void ui_hideAlert(void)
{
    if (uiState.alertType != UI_ALERT_NONE &&
        millis() - uiState.alertTime > ALERT_TIMEOUT_MS)
    {
        uiState.alertType = UI_ALERT_NONE;
        // Only clear the alert area, not redraw the whole screen
        draw_fillRect(0, CONTENT_Y, SCREEN_WIDTH, 30, COLOR_WHITE);
    }
}

// ===================================
// Screen Management
// ===================================

// Helper function to redraw a single navbar button
static void ui_drawNavbarButton(int index)
{
    const uint16_t navColors[] = {COLOR_BLUE, COLOR_GREEN, COLOR_ORANGE, COLOR_GRAY, COLOR_CYAN};
    const int navWidth = SCREEN_WIDTH / 5;

    if (index < 0 || index >= 5)
    {
        return;
    }

    int16_t btnX = index * navWidth;
    uint16_t bgColor = (uiState.currentScreen == index) ? navColors[index] : COLOR_DARKGRAY;
    uint16_t iconColor = (uiState.currentScreen == index) ? COLOR_WHITE : COLOR_LIGHTGRAY;

    // Button background
    draw_fillRect(btnX, NAVBAR_Y, navWidth, NAVBAR_HEIGHT, bgColor);

    // Draw icon from PROGMEM
    int16_t iconX = btnX + (navWidth - ICON_WIDTH) / 2;
    int16_t iconY = NAVBAR_Y + (NAVBAR_HEIGHT - ICON_HEIGHT) / 2;

    // Get the icon bitmap from the array
    const unsigned char *iconBitmap = (const unsigned char *)pgm_read_ptr(&navbar_icons[index]);
    draw_iconBitmap(iconX, iconY, iconBitmap, ICON_WIDTH, ICON_HEIGHT, iconColor);
}

void ui_setScreen(ScreenID screen)
{
    if (screen >= SCREEN_COUNT)
    {
        return;
    }
    if (screen == uiState.currentScreen)
    {
        return;
    }

    ScreenID oldScreen = uiState.currentScreen;
    uiState.lastScreen = uiState.currentScreen;
    uiState.currentScreen = screen;

    // Only redraw the two navbar buttons that changed (old and new)
    ui_drawNavbarButton(oldScreen); // Unhighlight old
    ui_drawNavbarButton(screen);    // Highlight new

    // Trigger content area redraw, but NOT navbar redraw (we just did that manually)
    uiState.needsFullRedraw = true;
    uiState.needsNavbarRedraw = false; // Navbar already updated above

    // Clear screen-specific data
    ui_clearValues();
    ui_clearButtons();
}

ScreenID ui_getCurrentScreen(void)
{
    return uiState.currentScreen;
}

ScreenID ui_getLastScreen(void)
{
    return uiState.lastScreen;
}

void ui_goBack(void)
{
    ui_setScreen(uiState.lastScreen);
}

void ui_requestRedraw(void)
{
    uiState.needsFullRedraw = true;
}

// ===================================
// Layout Drawing
// ===================================

void ui_drawHeader(const char *title)
{
    draw_fillRect(0, 0, SCREEN_WIDTH, HEADER_HEIGHT, COLOR_BLUE);

    // Draw status indicators within the header
    // Battery (left side)
    draw_battery(5, 10, uiState.batteryLevel);

    // GPS indicator
    draw_gpsIndicator(35, 10, uiState.gpsLock);

    // GSM signal (right side)
    draw_gsmSignal(SCREEN_WIDTH - 30, 8, uiState.gsmSignal);

    // Note: Title text rendering would require a font library
}

void ui_drawStatus(void)
{
    // Status bar is now integrated into header, so this function does nothing
    // Kept for compatibility
}

void ui_drawFooter(void)
{
    // Only draw when needsNavbarRedraw is set (initial load only)
    // Screen changes update navbar manually in ui_setScreen()
    if (!uiState.needsNavbarRedraw)
    {
        return;
    }

    draw_fillRect(0, NAVBAR_Y, SCREEN_WIDTH, NAVBAR_HEIGHT, COLOR_DARKGRAY);

    const uint16_t navColors[] = {COLOR_BLUE, COLOR_GREEN, COLOR_ORANGE, COLOR_GRAY, COLOR_CYAN};
    const int navWidth = SCREEN_WIDTH / 5;

    for (int i = 0; i < 5; i++)
    {
        int16_t btnX = i * navWidth;
        uint16_t bgColor = (uiState.currentScreen == i) ? navColors[i] : COLOR_DARKGRAY;
        uint16_t iconColor = (uiState.currentScreen == i) ? COLOR_WHITE : COLOR_LIGHTGRAY;

        // Button background
        draw_fillRect(btnX, NAVBAR_Y, navWidth, NAVBAR_HEIGHT, bgColor);

        // Draw icon from PROGMEM
        int16_t iconX = btnX + (navWidth - ICON_WIDTH) / 2;
        int16_t iconY = NAVBAR_Y + (NAVBAR_HEIGHT - ICON_HEIGHT) / 2;

        // Get the icon bitmap from the array
        const unsigned char *iconBitmap = (const unsigned char *)pgm_read_ptr(&navbar_icons[i]);
        draw_iconBitmap(iconX, iconY, iconBitmap, ICON_WIDTH, ICON_HEIGHT, iconColor);
    }

    // Clear the flag after drawing
    uiState.needsNavbarRedraw = false;
}

void ui_drawScreen(void)
{
    if (!uiState.needsFullRedraw)
    {
        return;
    }

    // Draw header and status only on full redraw
    ui_drawHeader("Farm Monitor");
    ui_drawStatus();

    // Draw screen content based on current screen
    switch (uiState.currentScreen)
    {
    case SCREEN_HOME:
        screen_home_draw();
        break;
    case SCREEN_FILES:
        screen_files_draw();
        break;
    case SCREEN_AI:
        screen_ai_draw();
        break;
    case SCREEN_SETTINGS:
        screen_settings_draw();
        break;
    case SCREEN_INPUT:
        screen_input_draw();
        break;
    default:
        break;
    }

    ui_drawFooter();
    uiState.needsFullRedraw = false;
}

// ===================================
// Status Updates
// ===================================

void ui_setGSM(uint8_t signal)
{
    if (uiState.gsmSignal != signal)
    {
        uiState.gsmSignal = signal;
        // Only redraw the GSM signal area within the header
        draw_fillRect(SCREEN_WIDTH - 35, 0, 35, HEADER_HEIGHT, COLOR_BLUE);
        draw_gsmSignal(SCREEN_WIDTH - 30, 8, uiState.gsmSignal);
    }
}

void ui_setBattery(uint8_t level)
{
    if (uiState.batteryLevel != level)
    {
        uiState.batteryLevel = level;
        // Only redraw the battery area within the header
        draw_fillRect(0, 0, 30, HEADER_HEIGHT, COLOR_BLUE);
        draw_battery(5, 10, uiState.batteryLevel);
    }
}

void ui_setGPS(bool locked)
{
    if (uiState.gpsLock != locked)
    {
        uiState.gpsLock = locked;
        // Only redraw the GPS indicator area within the header
        draw_fillRect(30, 0, 20, HEADER_HEIGHT, COLOR_BLUE);
        draw_gpsIndicator(35, 10, uiState.gpsLock);
    }
}

// ===================================
// Touch Handling
// ===================================

void ui_handleNavbar(int16_t x, int16_t y)
{
    int navWidth = SCREEN_WIDTH / 5;
    int navIndex = x / navWidth;

    if (navIndex >= 0 && navIndex < SCREEN_COUNT)
    {
        ui_setScreen((ScreenID)navIndex);
    }
}

void ui_handleTouch(int16_t x, int16_t y)
{
    // Debounce
    if (millis() - uiState.lastTouchTime < TOUCH_DEBOUNCE_MS)
    {
        return;
    }
    uiState.lastTouchTime = millis();
    uiState.lastTouchX = x;
    uiState.lastTouchY = y;

    // Check navbar first
    if (y >= NAVBAR_Y)
    {
        ui_handleNavbar(x, y);
        return;
    }

    // Screen-specific touch handling
    if (uiState.currentScreen == SCREEN_FILES)
    {
        screen_files_handleTouch(x, y);
        return;
    }

    // Check buttons for other screens
    for (int i = 0; i < buttonCount; i++)
    {
        if (ui_checkButton(&buttons[i], x, y))
        {
            if (buttons[i].callback != NULL)
            {
                buttons[i].callback();
            }
            return;
        }
    }

    // Screen-specific touch handling could be added here for other screens
}

// ===================================
// Main Update Loop
// ===================================

void ui_update(void)
{
    // Handle touch input
    int16_t x, y;
    if (touch_getPoint(&x, &y))
    {
        ui_handleTouch(x, y);
    }

    // Update screen if needed
    ui_drawScreen();

    // Update dynamic values
    ui_redrawValues();

    // Hide alert after timeout
    ui_hideAlert();
}