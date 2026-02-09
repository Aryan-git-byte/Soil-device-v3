/**
 * @file tft_driver.h
 * @brief Bare Metal TFT Display Driver for ILI9341 on SAMD21
 *
 * Uses SERCOM1 hardware SPI with direct register access.
 * No external libraries required.
 */

#ifndef TFT_DRIVER_H
#define TFT_DRIVER_H

#include <Arduino.h>
#include "config.h"

// ===================================
// SPI Bus Functions (SERCOM1)
// ===================================

/**
 * @brief Initialize SERCOM1 as hardware SPI master
 *
 * Configures PA16 (MOSI), PA17 (SCK), PA19 (MISO) via SERCOM1.
 * SPI clock: 12 MHz (48 MHz / 4).
 */
void spi_init(void);

/**
 * @brief Transfer one byte over hardware SPI
 * @param data Byte to send
 * @return Byte received from slave
 */
uint8_t spi_transfer(uint8_t data);

// ===================================
// TFT Driver Functions
// ===================================

/**
 * @brief Initialize the ILI9341 display
 *
 * Configures control pin directions, performs hardware reset,
 * and sends the full ILI9341 initialization sequence.
 * spi_init() must be called before this function.
 */
void tft_init(void);

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
 * @brief Set the drawing address window
 * @param x0 Start X coordinate
 * @param y0 Start Y coordinate
 * @param x1 End X coordinate
 * @param y1 End Y coordinate
 */
void tft_setWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

/**
 * @brief Begin bulk pixel data write (DC=HIGH, CS=LOW)
 */
void tft_beginWrite(void);

/**
 * @brief End bulk pixel data write (CS=HIGH)
 */
void tft_endWrite(void);

/**
 * @brief Write a 16-bit color during bulk write (no CS/DC toggling)
 * @param color RGB565 color
 */
void tft_writeColor(uint16_t color);

#endif // TFT_DRIVER_H
