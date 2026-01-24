/**
 * @file tft_driver.cpp
 * @brief TFT Display Driver Implementation for ILI9341
 */

#include "tft_driver.h"
#include <SPI.h>

// ===================================
// TFT Low Level SPI (Hardware SPI)
// ===================================

void tft_spiWrite(uint8_t data)
{
    SPI.transfer(data);
}

void tft_writeCommand(uint8_t cmd)
{
    SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
    digitalWrite(TFT_DC, LOW);
    digitalWrite(TFT_CS, LOW);
    SPI.transfer(cmd);
    digitalWrite(TFT_CS, HIGH);
    SPI.endTransaction();
}

void tft_writeData(uint8_t data)
{
    SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
    digitalWrite(TFT_DC, HIGH);
    digitalWrite(TFT_CS, LOW);
    SPI.transfer(data);
    digitalWrite(TFT_CS, HIGH);
    SPI.endTransaction();
}

void tft_writeData16(uint16_t data)
{
    SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
    digitalWrite(TFT_DC, HIGH);
    digitalWrite(TFT_CS, LOW);
    SPI.transfer(data >> 8);
    SPI.transfer(data & 0xFF);
    digitalWrite(TFT_CS, HIGH);
    SPI.endTransaction();
}

// ===================================
// Window Management
// ===================================

void tft_setWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    tft_writeCommand(ILI9341_CASET);
    tft_writeData16(x0);
    tft_writeData16(x1);
    tft_writeCommand(ILI9341_PASET);
    tft_writeData16(y0);
    tft_writeData16(y1);
    tft_writeCommand(ILI9341_RAMWR);
}

void tft_beginWrite(void)
{
    SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
    digitalWrite(TFT_DC, HIGH);
    digitalWrite(TFT_CS, LOW);
}

void tft_endWrite(void)
{
    digitalWrite(TFT_CS, HIGH);
    SPI.endTransaction();
}

void tft_writeColor(uint16_t color)
{
    SPI.transfer(color >> 8);
    SPI.transfer(color & 0xFF);
}

// ===================================
// Initialization
// ===================================

void tft_initPins(void)
{
    // Initialize control pins first
    pinMode(TFT_CS, OUTPUT);
    pinMode(TFT_RST, OUTPUT);
    pinMode(TFT_DC, OUTPUT);

    digitalWrite(TFT_CS, HIGH);
    digitalWrite(TFT_DC, HIGH);

    // Initialize hardware SPI
    SPI.begin();
    // Arduino Zero runs at 48MHz, use a safe divider
    // Try starting with slower speed for stability
    SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0)); // 8MHz
    SPI.endTransaction();
}

void tft_init(void)
{
    // Hardware reset
    digitalWrite(TFT_RST, HIGH);
    delay(10);
    digitalWrite(TFT_RST, LOW);
    delay(20);
    digitalWrite(TFT_RST, HIGH);
    delay(150);

    // Software reset
    tft_writeCommand(ILI9341_SWRESET);
    delay(150);
    tft_writeCommand(ILI9341_SLPOUT);
    delay(120);

    // Power control sequences
    tft_writeCommand(0xCB);
    tft_writeData(0x39);
    tft_writeData(0x2C);
    tft_writeData(0x00);
    tft_writeData(0x34);
    tft_writeData(0x02);

    tft_writeCommand(0xCF);
    tft_writeData(0x00);
    tft_writeData(0xC1);
    tft_writeData(0x30);

    tft_writeCommand(0xE8);
    tft_writeData(0x85);
    tft_writeData(0x00);
    tft_writeData(0x78);

    tft_writeCommand(0xEA);
    tft_writeData(0x00);
    tft_writeData(0x00);

    tft_writeCommand(0xED);
    tft_writeData(0x64);
    tft_writeData(0x03);
    tft_writeData(0x12);
    tft_writeData(0x81);

    tft_writeCommand(0xF7);
    tft_writeData(0x20);

    // Power control
    tft_writeCommand(ILI9341_PWCTR1);
    tft_writeData(0x23);

    tft_writeCommand(ILI9341_PWCTR2);
    tft_writeData(0x10);

    // VCOM control
    tft_writeCommand(ILI9341_VMCTR1);
    tft_writeData(0x3E);
    tft_writeData(0x28);

    tft_writeCommand(ILI9341_VMCTR2);
    tft_writeData(0x86);

    // Memory access control (rotation)
    tft_writeCommand(ILI9341_MADCTL);
    tft_writeData(0x68);

    // Pixel format
    tft_writeCommand(ILI9341_PIXFMT);
    tft_writeData(0x55);

    // Frame rate control
    tft_writeCommand(ILI9341_FRMCTR1);
    tft_writeData(0x00);
    tft_writeData(0x10);

    // Display function control
    tft_writeCommand(0xB6);
    tft_writeData(0x08);
    tft_writeData(0x82);
    tft_writeData(0x27);

    // 3Gamma function disable
    tft_writeCommand(0xF2);
    tft_writeData(0x00);

    // Gamma curve selected
    tft_writeCommand(0x26);
    tft_writeData(0x01);

    // Positive gamma correction
    tft_writeCommand(0xE0);
    tft_writeData(0x0F);
    tft_writeData(0x31);
    tft_writeData(0x2B);
    tft_writeData(0x0C);
    tft_writeData(0x0E);
    tft_writeData(0x08);
    tft_writeData(0x4E);
    tft_writeData(0xF1);
    tft_writeData(0x37);
    tft_writeData(0x07);
    tft_writeData(0x10);
    tft_writeData(0x03);
    tft_writeData(0x0E);
    tft_writeData(0x09);
    tft_writeData(0x00);

    // Negative gamma correction
    tft_writeCommand(0xE1);
    tft_writeData(0x00);
    tft_writeData(0x0E);
    tft_writeData(0x14);
    tft_writeData(0x03);
    tft_writeData(0x11);
    tft_writeData(0x07);
    tft_writeData(0x31);
    tft_writeData(0xC1);
    tft_writeData(0x48);
    tft_writeData(0x08);
    tft_writeData(0x0F);
    tft_writeData(0x0C);
    tft_writeData(0x31);
    tft_writeData(0x36);
    tft_writeData(0x0F);

    // Display on
    tft_writeCommand(ILI9341_DISPON);
    delay(100);
}
