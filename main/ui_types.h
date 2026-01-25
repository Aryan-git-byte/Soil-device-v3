/**
 * @file ui_types.h
 * @brief Type definitions and structures for the UI Engine
 */

#ifndef UI_TYPES_H
#define UI_TYPES_H

#include <Arduino.h>
#include "config.h"

// ===================================
// Screen IDs Enumeration
// ===================================
typedef enum
{
    SCREEN_HOME = 0,
    SCREEN_FILES,
    SCREEN_AI,
    SCREEN_SETTINGS,
    SCREEN_INPUT,
    SCREEN_COUNT
} ScreenID;

// ===================================
// Alert Types Enumeration
// ===================================
typedef enum
{
    UI_ALERT_NONE = 0,
    UI_ALERT_INFO,
    UI_ALERT_WARN,
    UI_ALERT_ERROR
} AlertType;

// ===================================
// Label IDs for Bilingual Support
// ===================================
typedef enum
{
    LABEL_MOISTURE = 0,
    LABEL_NITROGEN,
    LABEL_PHOSPHORUS,
    LABEL_POTASSIUM,
    LABEL_TEMPERATURE,
    LABEL_HUMIDITY,
    LABEL_PH,
    LABEL_COUNT
} LabelID;

// ===================================
// Button Structure
// ===================================
typedef struct
{
    int16_t x;
    int16_t y;
    int16_t w;
    int16_t h;
    const char *label;
    uint16_t color;
    void (*callback)(void);
    bool visible;
} UIButton;

// ===================================
// Data Binding Structure
// ===================================
typedef struct
{
    LabelID id;
    int16_t x;
    int16_t y;
    int16_t value;
    int16_t lastValue;
    bool needsRedraw;
} UIValue;

// ===================================
// Touch Point Structure
// ===================================
typedef struct
{
    int16_t x;
    int16_t y;
    bool pressed;
} TouchPoint;

// ===================================
// UI State Structure
// ===================================
typedef struct
{
    ScreenID currentScreen;
    ScreenID lastScreen;
    AlertType alertType;
    char alertMsg[MAX_ALERT_LEN];
    uint32_t alertTime;
    bool needsFullRedraw;
    bool needsNavbarRedraw; // Separate flag for navbar
    int16_t lastTouchX;
    int16_t lastTouchY;
    uint32_t lastTouchTime;
    uint8_t gsmSignal;
    uint8_t batteryLevel;
    bool gpsLock;
    float gpsLatitude;
    float gpsLongitude;
    bool gpsValid;
} UIState;

// ===================================
// Language Strings
// ===================================
extern const char *labels_en[LABEL_COUNT];

#endif // UI_TYPES_H
