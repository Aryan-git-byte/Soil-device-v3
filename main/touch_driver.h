/**
 * @file touch_driver.h
 * @brief Touch Screen Driver for XPT2046
 *
 * Low-level SPI communication and touch coordinate handling
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
 * @brief Initialize touch controller GPIO pins
 */
void touch_initPins(void);

/**
 * @brief Transfer a byte via software SPI
 * @param data Byte to send
 * @return Received byte
 */
uint8_t touch_spiTransfer(uint8_t data);

/**
 * @brief Read a value from the touch controller
 * @param command Touch controller command
 * @return 12-bit ADC value
 */
uint16_t touch_read(uint8_t command);

/**
 * @brief Get raw touch coordinates
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
 * @brief Check if screen is currently being touched
 * @return true if touch detected
 */
bool touch_isTouched(void);

#endif // TOUCH_DRIVER_H
