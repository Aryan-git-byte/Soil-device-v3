/**
 * @file tft_driver.h
 * @brief TFT Display Driver for ILI9341
 *
 * Low-level SPI communication and display initialization
 */

#ifndef TFT_DRIVER_H
#define TFT_DRIVER_H

#include <Arduino.h>
#include "config.h"

// ===================================
// TFT Driver Functions
// ===================================

/**
 * @brief Initialize TFT GPIO pins
 */
void tft_initPins(void);

/**
 * @brief Initialize the ILI9341 display
 */
void tft_init(void);

/**
 * @brief Write a single byte via software SPI
 * @param data Byte to write
 */
void tft_spiWrite(uint8_t data);

/**
 * @brief Write a command byte to the display
 * @param cmd Command byte
 */
void tft_writeCommand(uint8_t cmd);

/**
 * @brief Write a data byte to the display
 * @param data Data byte
 */
void tft_writeData(uint8_t data);

/**
 * @brief Write 16-bit data to the display
 * @param data 16-bit data value
 */
void tft_writeData16(uint16_t data);

/**
 * @brief Set the drawing window
 * @param x0 Start X coordinate
 * @param y0 Start Y coordinate
 * @param x1 End X coordinate
 * @param y1 End Y coordinate
 */
void tft_setWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

/**
 * @brief Begin bulk data write (call after setWindow)
 */
void tft_beginWrite(void);

/**
 * @brief End bulk data write
 */
void tft_endWrite(void);

/**
 * @brief Write a 16-bit color during bulk write
 * @param color RGB565 color
 */
void tft_writeColor(uint16_t color);

#endif // TFT_DRIVER_H
