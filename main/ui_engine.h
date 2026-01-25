/**
 * @file ui_engine.h
 * @brief UI Engine - Core UI Management System
 *
 * Button system, data binding, alerts, screen management, and touch routing
 */

#ifndef UI_ENGINE_H
#define UI_ENGINE_H

#include <Arduino.h>
#include "config.h"
#include "ui_types.h"

// ===================================
// Global State Access
// ===================================

/**
 * @brief Get the global UI state
 * @return Pointer to UI state structure
 */
UIState *ui_getState(void);

// ===================================
// UI Engine Initialization
// ===================================

/**
 * @brief Initialize the UI engine
 */
void ui_init(void);

// ===================================
// Button System
// ===================================

/**
 * @brief Add a button to the current screen
 * @param x X coordinate
 * @param y Y coordinate
 * @param w Width
 * @param h Height
 * @param label Button label
 * @param color Button color
 * @param callback Function to call on press
 * @return Button index or -1 if failed
 */
int ui_addButton(int16_t x, int16_t y, int16_t w, int16_t h,
                 const char *label, uint16_t color, void (*callback)(void));

/**
 * @brief Clear all buttons
 */
void ui_clearButtons(void);

/**
 * @brief Check if a point is within a button
 * @param btn Pointer to button
 * @param x X coordinate
 * @param y Y coordinate
 * @return true if point is inside button
 */
bool ui_checkButton(UIButton *btn, int16_t x, int16_t y);

/**
 * @brief Get button by index
 * @param index Button index
 * @return Pointer to button or NULL
 */
UIButton *ui_getButton(int index);

/**
 * @brief Get current button count
 * @return Number of active buttons
 */
int ui_getButtonCount(void);

// ===================================
// Data Binding System
// ===================================

/**
 * @brief Register a value for display and updates
 * @param id Label ID
 * @param x X coordinate
 * @param y Y coordinate
 * @param initialValue Initial display value
 */
void ui_registerValue(LabelID id, int16_t x, int16_t y, int16_t initialValue);

/**
 * @brief Update a registered value
 * @param id Label ID
 * @param newValue New value to display
 */
void ui_updateValue(LabelID id, int16_t newValue);

/**
 * @brief Redraw values that have changed
 */
void ui_redrawValues(void);

/**
 * @brief Clear all registered values
 */
void ui_clearValues(void);

// ===================================
// Alert System
// ===================================

/**
 * @brief Show an alert message
 * @param msg Alert message text
 * @param type Alert type (info/warn/error)
 */
void ui_showAlert(const char *msg, AlertType type);

/**
 * @brief Hide the current alert (called automatically after timeout)
 */
void ui_hideAlert(void);

/**
 * @brief Draw an alert banner
 * @param text Alert text
 * @param type Alert type
 */
void ui_drawAlert(const char *text, AlertType type);

// ===================================
// Screen Management
// ===================================

/**
 * @brief Switch to a different screen
 * @param screen Screen ID to switch to
 */
void ui_setScreen(ScreenID screen);

/**
 * @brief Get the current screen ID
 * @return Current screen ID
 */
ScreenID ui_getCurrentScreen(void);

/**
 * @brief Get the previous screen ID
 * @return Previous screen ID
 */
ScreenID ui_getLastScreen(void);

/**
 * @brief Go back to the previous screen
 */
void ui_goBack(void);

/**
 * @brief Request a full screen redraw
 */
void ui_requestRedraw(void);

/**
 * @brief Draw the current screen (if needed)
 */
void ui_drawScreen(void);

// ===================================
// Layout Drawing
// ===================================

/**
 * @brief Draw the header bar
 * @param title Header title
 */
void ui_drawHeader(const char *title);

/**
 * @brief Draw the status bar
 */
void ui_drawStatus(void);

/**
 * @brief Draw the navigation footer
 */
void ui_drawFooter(void);

// ===================================
// Status Updates
// ===================================

/**
 * @brief Update GSM signal display
 * @param signal Signal strength (0-100)
 */
void ui_setGSM(uint8_t signal);

/**
 * @brief Update battery level display
 * @param level Battery level (0-100)
 */
void ui_setBattery(uint8_t level);

/**
 * @brief Update GPS lock status display
 * @param locked GPS lock status
 */
void ui_setGPS(bool locked);

/**
 * @brief Update GPS coordinates display
 * @param lat Latitude
 * @param lon Longitude
 * @param valid GPS data valid
 */
void ui_setGPSCoordinates(float lat, float lon, bool valid);

// ===================================
// Touch Handling
// ===================================

/**
 * @brief Handle a touch event at the given coordinates
 * @param x X coordinate
 * @param y Y coordinate
 */
void ui_handleTouch(int16_t x, int16_t y);

/**
 * @brief Handle navbar touch
 * @param x X coordinate
 * @param y Y coordinate
 */
void ui_handleNavbar(int16_t x, int16_t y);

// ===================================
// Main Update Loop
// ===================================

/**
 * @brief Main UI update function - call this in loop()
 */
void ui_update(void);

#endif // UI_ENGINE_H
