/**
 * @file touch_driver.cpp
 * @brief Bare Metal Touch Screen Driver Implementation for XPT2046 on SAMD21
 *
 * Shares SERCOM1 hardware SPI with the TFT driver.
 * Uses direct port manipulation for TOUCH_CS pin.
 */

#include "touch_driver.h"
#include "tft_driver.h" // for spi_transfer()

// ===================================
// Touch Low-Level SPI
// ===================================

uint16_t touch_readRaw(uint8_t cmd)
{
    CLR_PIN(TOUCH_CS_PORT, TOUCH_CS_PIN); // Select touch controller
    spi_transfer(cmd);
    uint8_t hi = spi_transfer(0x00);
    uint8_t lo = spi_transfer(0x00);
    SET_PIN(TOUCH_CS_PORT, TOUCH_CS_PIN); // Deselect touch controller
    return ((hi << 8) | lo) >> 3;         // 12-bit result
}

// ===================================
// Touch Detection
// ===================================

bool touch_isTouched(void)
{
    // Read Z1 pressure to detect touch
    uint16_t z = touch_readRaw(XPT2046_CMD_Z1);
    return (z > PRESSURE_THRESHOLD);
}

bool touch_getRaw(uint16_t *x, uint16_t *y, uint16_t *z)
{
    // Read pressure first for quick rejection
    uint16_t z1 = touch_readRaw(XPT2046_CMD_Z1);
    if (z1 < PRESSURE_THRESHOLD)
    {
        return false;
    }

    // Take multiple samples for averaging
    const uint8_t samples = 4;
    uint32_t sumX = 0, sumY = 0, sumZ1 = 0, sumZ2 = 0;

    for (uint8_t i = 0; i < samples; i++)
    {
        sumX += touch_readRaw(XPT2046_CMD_X);
        sumY += touch_readRaw(XPT2046_CMD_Y);
        sumZ1 += touch_readRaw(XPT2046_CMD_Z1);
        sumZ2 += touch_readRaw(XPT2046_CMD_Z2);
    }

    *x = sumX / samples;
    *y = sumY / samples;

    uint16_t avgZ1 = sumZ1 / samples;
    uint16_t avgZ2 = sumZ2 / samples;

    // Calculate pressure
    if (avgZ1 == 0)
    {
        *z = 0;
    }
    else
    {
        *z = (*x * (avgZ2 - avgZ1)) / avgZ1;
    }

    return (*z > PRESSURE_THRESHOLD);
}

bool touch_getPoint(int16_t *x, int16_t *y)
{
    uint16_t rawX, rawY, pressure;

    if (!touch_getRaw(&rawX, &rawY, &pressure))
    {
        return false;
    }

    // Map raw ADC values to screen coordinates
    *x = map(rawX, TS_MINX, TS_MAXX, 0, SCREEN_WIDTH);
    *y = map(rawY, TS_MAXY, TS_MINY, 0, SCREEN_HEIGHT);

    // Constrain to screen bounds
    *x = constrain(*x, 0, SCREEN_WIDTH - 1);
    *y = constrain(*y, 0, SCREEN_HEIGHT - 1);

    return true;
}