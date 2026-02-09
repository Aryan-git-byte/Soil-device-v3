/**
 * @file touch_driver.h
 * @brief Bare Metal Touch Screen Driver for XPT2046 on SAMD21
 *
 * Shares SERCOM1 hardware SPI bus with ILI9341 TFT driver.
 * Uses direct port register manipulation for CS pin control.
 */

#ifndef TOUCH_DRIVER_H
#define TOUCH_DRIVER_H

#include <Arduino.h>
#include "config.h"
#include "ui_types.h"

// ===================================
// Touch Driver Functions
// ===================================

/**
 * @brief Read a raw 12-bit ADC value from the XPT2046
 * @param cmd XPT2046 command byte (e.g. 0xD0 for X, 0x90 for Y)
 * @return 12-bit ADC value
 */
uint16_t touch_readRaw(uint8_t cmd);

/**
 * @brief Get raw touch coordinates with averaging
 * @param x Pointer to store raw X value
 * @param y Pointer to store raw Y value
 * @param z Pointer to store pressure value
 * @return true if touch detected with sufficient pressure
 */
bool touch_getRaw(uint16_t *x, uint16_t *y, uint16_t *z);

/**
 * @brief Get calibrated screen coordinates
 * @param x Pointer to store screen X coordinate
 * @param y Pointer to store screen Y coordinate
 * @return true if valid touch detected
 */
bool touch_getPoint(int16_t *x, int16_t *y);

/**
 * @brief Check if screen is currently being touched (pressure-based)
 * @return true if touch detected
 */
bool touch_isTouched(void);

#endif // TOUCH_DRIVER_H
