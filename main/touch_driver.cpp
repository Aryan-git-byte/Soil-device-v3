/**
 * @file touch_driver.cpp
 * @brief Touch Screen Driver Implementation for XPT2046
 */

#include "touch_driver.h"

// ===================================
// Touch Low Level SPI
// ===================================

void touch_initPins(void)
{
    pinMode(T_CS, OUTPUT);
    pinMode(T_IRQ, INPUT);
    pinMode(T_DIN, OUTPUT);
    pinMode(T_DO, INPUT);
    pinMode(T_CLK, OUTPUT);

    digitalWrite(T_CS, HIGH);
    digitalWrite(T_CLK, LOW);
}

uint8_t touch_spiTransfer(uint8_t data)
{
    uint8_t reply = 0;
    for (int8_t i = 7; i >= 0; i--)
    {
        digitalWrite(T_CLK, LOW);
        digitalWrite(T_DIN, (data >> i) & 0x01);
        digitalWrite(T_CLK, HIGH);
        reply <<= 1;
        if (digitalRead(T_DO))
        {
            reply |= 1;
        }
    }
    return reply;
}

uint16_t touch_read(uint8_t command)
{
    digitalWrite(T_CS, LOW);
    touch_spiTransfer(command);
    uint8_t high = touch_spiTransfer(0x00);
    uint8_t low = touch_spiTransfer(0x00);
    digitalWrite(T_CS, HIGH);
    return ((high << 8) | low) >> 3;
}

// ===================================
// Touch Detection
// ===================================

bool touch_isTouched(void)
{
    return (digitalRead(T_IRQ) == LOW);
}

bool touch_getRaw(uint16_t *x, uint16_t *y, uint16_t *z)
{
    if (!touch_isTouched())
    {
        return false;
    }

    // Take multiple samples for averaging
    const uint8_t samples = 4;
    uint32_t sumX = 0, sumY = 0, sumZ1 = 0, sumZ2 = 0;

    for (uint8_t i = 0; i < samples; i++)
    {
        sumX += touch_read(XPT2046_CMD_X);
        sumY += touch_read(XPT2046_CMD_Y);
        sumZ1 += touch_read(XPT2046_CMD_Z1);
        sumZ2 += touch_read(XPT2046_CMD_Z2);
    }

    *x = sumX / samples;
    *y = sumY / samples;

    uint16_t z1 = sumZ1 / samples;
    uint16_t z2 = sumZ2 / samples;

    // Calculate pressure
    if (z1 == 0)
    {
        *z = 0;
    }
    else
    {
        *z = (*x * (z2 - z1)) / z1;
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

    // Map raw values to screen coordinates
    // FIXED: Swapped TS_MINX and TS_MAXX to flip horizontally
    *x = map(rawX, TS_MINX, TS_MAXX, 0, SCREEN_WIDTH);
    *y = map(rawY, TS_MAXY, TS_MINY, 0, SCREEN_HEIGHT);

    // Constrain to screen bounds
    *x = constrain(*x, 0, SCREEN_WIDTH - 1);
    *y = constrain(*y, 0, SCREEN_HEIGHT - 1);

    return true;
}