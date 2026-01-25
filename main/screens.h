/**
 * @file screens.h
 * @brief Screen Implementations
 *
 * Individual screen drawing functions
 */

#ifndef SCREENS_H
#define SCREENS_H

#include <Arduino.h>
#include "config.h"
#include "ui_types.h"

// ===================================
// Screen Drawing Functions
// ===================================

/**
 * @brief Draw the Home screen (sensor dashboard)
 */
void screen_home_draw(void);

/**
 * @brief Draw the Files screen
 */
void screen_files_draw(void);

/**
 * @brief Handle touch events on Files screen
 * @param x X coordinate
 * @param y Y coordinate
 */
void screen_files_handleTouch(int16_t x, int16_t y);

/**
 * @brief Draw the AI screen
 */
void screen_ai_draw(void);

/**
 * @brief Draw the Settings screen
 */
void screen_settings_draw(void);

/**
 * @brief Draw the Input screen (keyboard)
 */
void screen_input_draw(void);

#endif // SCREENS_H