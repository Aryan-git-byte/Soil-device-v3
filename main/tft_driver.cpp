/**
 * @file tft_driver.cpp
 * @brief Bare Metal TFT Driver Implementation for ILI9341 on SAMD21
 *
 * Uses SERCOM1 hardware SPI with direct port register manipulation.
 * No external libraries. ~12 MHz SPI clock.
 */

#include "tft_driver.h"

// ===================================
// Bare Metal SPI (SERCOM1)
// ===================================

void spi_init(void)
{
    // 1. Enable power to SERCOM1
    PM->APBCMASK.reg |= PM_APBCMASK_SERCOM1;

    // 2. Configure GCLK0 (48 MHz) to feed SERCOM1
    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(SERCOM1_GCLK_ID_CORE) |
                        GCLK_CLKCTRL_GEN_GCLK0 |
                        GCLK_CLKCTRL_CLKEN;
    while (GCLK->STATUS.bit.SYNCBUSY)
        ;

    // 3. Configure pins for SERCOM1 (Peripheral Function C)
    // PA16 (MOSI) -> Pad 0
    // PA17 (SCK)  -> Pad 1
    // PA19 (MISO) -> Pad 3
    PORT->Group[0].PINCFG[16].bit.PMUXEN = 1;
    PORT->Group[0].PINCFG[17].bit.PMUXEN = 1;
    PORT->Group[0].PINCFG[19].bit.PMUXEN = 1;

    // PMUX register: even pins use lower nibble, odd pins use upper nibble
    // PA16 (even) & PA17 (odd) share PMUX[8]
    PORT->Group[0].PMUX[8].reg = PORT_PMUX_PMUXO_C | PORT_PMUX_PMUXE_C;
    // PA19 (odd) in PMUX[9]
    PORT->Group[0].PMUX[9].reg |= PORT_PMUX_PMUXO_C;

    // 4. Configure SERCOM1 as SPI Master
    SERCOM1->SPI.CTRLA.bit.ENABLE = 0;
    while (SERCOM1->SPI.SYNCBUSY.bit.ENABLE)
        ;

    SERCOM1->SPI.CTRLA.reg = SERCOM_SPI_CTRLA_MODE_SPI_MASTER |
                             SERCOM_SPI_CTRLA_DIPO(3) | // MISO on Pad 3
                             SERCOM_SPI_CTRLA_DOPO(0);  // MOSI Pad 0, SCK Pad 1

    SERCOM1->SPI.CTRLB.reg = SERCOM_SPI_CTRLB_RXEN; // Enable receiver

    // 5. Set baud rate: 12 MHz SPI clock -> (48 / (2*12)) - 1 = 1
    SERCOM1->SPI.BAUD.reg = 1;

    // 6. Enable SPI
    SERCOM1->SPI.CTRLA.bit.ENABLE = 1;
    while (SERCOM1->SPI.SYNCBUSY.bit.ENABLE)
        ;
}

uint8_t spi_transfer(uint8_t data)
{
    while (SERCOM1->SPI.INTFLAG.bit.DRE == 0)
        ; // Wait for Data Register Empty
    SERCOM1->SPI.DATA.reg = data;
    while (SERCOM1->SPI.INTFLAG.bit.RXC == 0)
        ; // Wait for Receive Complete
    return (uint8_t)SERCOM1->SPI.DATA.reg;
}

// ===================================
// TFT Low-Level Commands
// ===================================

void tft_writeCommand(uint8_t cmd)
{
    CLR_PIN(TFT_DC_PORT, TFT_DC_PIN); // DC Low  = Command
    CLR_PIN(TFT_CS_PORT, TFT_CS_PIN); // CS Low
    spi_transfer(cmd);
    SET_PIN(TFT_CS_PORT, TFT_CS_PIN); // CS High
}

void tft_writeData(uint8_t data)
{
    SET_PIN(TFT_DC_PORT, TFT_DC_PIN); // DC High = Data
    CLR_PIN(TFT_CS_PORT, TFT_CS_PIN);
    spi_transfer(data);
    SET_PIN(TFT_CS_PORT, TFT_CS_PIN);
}

void tft_writeData16(uint16_t data)
{
    SET_PIN(TFT_DC_PORT, TFT_DC_PIN);
    CLR_PIN(TFT_CS_PORT, TFT_CS_PIN);
    spi_transfer(data >> 8);
    spi_transfer(data & 0xFF);
    SET_PIN(TFT_CS_PORT, TFT_CS_PIN);
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
    SET_PIN(TFT_DC_PORT, TFT_DC_PIN); // DC High = Data
    CLR_PIN(TFT_CS_PORT, TFT_CS_PIN); // CS Low
}

void tft_endWrite(void)
{
    SET_PIN(TFT_CS_PORT, TFT_CS_PIN); // CS High
}

void tft_writeColor(uint16_t color)
{
    spi_transfer(color >> 8);
    spi_transfer(color & 0xFF);
}

// ===================================
// Initialization
// ===================================

void tft_init(void)
{
    // Configure control pins as outputs (bare metal)
    PIN_OUTPUT(TFT_CS_PORT, TFT_CS_PIN);
    PIN_OUTPUT(TFT_DC_PORT, TFT_DC_PIN);
    PIN_OUTPUT(TFT_RST_PORT, TFT_RST_PIN);
    PIN_OUTPUT(TOUCH_CS_PORT, TOUCH_CS_PIN);

    // Set defaults: all CS high, RST high
    SET_PIN(TFT_CS_PORT, TFT_CS_PIN);
    SET_PIN(TOUCH_CS_PORT, TOUCH_CS_PIN);
    SET_PIN(TFT_RST_PORT, TFT_RST_PIN);

    // Hardware reset
    delay(10);
    CLR_PIN(TFT_RST_PORT, TFT_RST_PIN);
    delay(10);
    SET_PIN(TFT_RST_PORT, TFT_RST_PIN);
    delay(150);

    // Software reset
    tft_writeCommand(ILI9341_SWRESET);
    delay(150);

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

    // Memory access control - Landscape 320x240
    // 0x28 = MV=1, BGR=1 -> landscape, normal orientation
    tft_writeCommand(ILI9341_MADCTL);
    tft_writeData(0x28);

    // Pixel format: 16-bit (RGB565)
    tft_writeCommand(ILI9341_PIXFMT);
    tft_writeData(0x55);

    // Frame rate control
    tft_writeCommand(ILI9341_FRMCTR1);
    tft_writeData(0x00);
    tft_writeData(0x18);

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

    // Sleep out and display on
    tft_writeCommand(ILI9341_SLPOUT);
    delay(120);
    tft_writeCommand(ILI9341_DISPON);
    delay(100);
}