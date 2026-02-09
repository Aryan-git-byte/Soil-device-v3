/**
 * @file main.ino
 * @brief FarmBot UI Engine - Main Arduino Sketch
 *
 * Arduino Zero based UI system with ILI9341 TFT and XPT2046 Touch
 *
 * Hardware:
 *   - Arduino Zero (SAMD21)
 *   - ILI9341 320x240 TFT Display
 *   - XPT2046 Resistive Touch Controller
 *
 * @author FarmBot Team
 * @version 3.0
 */

// Include all engine modules
#include "config.h"
#include "ui_types.h"
#include "tft_driver.h"
#include "touch_driver.h"
#include "drawing.h"
#include "ui_engine.h"
#include "screens.h"
#include "file_browser.h"
#include "a9g_gps.h"

// ===================================
// Application Configuration
// ===================================

// Sensor update interval in milliseconds
#define APP_SENSOR_UPDATE_INTERVAL 2000

// SD Card CS Pin - CHANGE THIS TO YOUR ACTUAL SD CARD CS PIN
#define SD_CS_PIN 10 // Common default, adjust if different

// ===================================
// Application State
// ===================================
FileBrowser sdBrowser;
A9G_GPS gpsModule;

static uint32_t lastSensorUpdate = 0;
static uint32_t lastGPSUpdate = 0;

// ===================================
// Sensor Simulation
// (Replace with actual sensor readings)
// ===================================

void updateSensorValues(void)
{
    // Simulate sensor readings with random values
    // In production, replace with actual sensor code
    ui_updateValue(LABEL_MOISTURE, random(20, 80));
    ui_updateValue(LABEL_NITROGEN, random(30, 90));
    ui_updateValue(LABEL_PHOSPHORUS, random(25, 75));
    ui_updateValue(LABEL_POTASSIUM, random(35, 85));
}

// ===================================
// SD Card Initialization with Debugging
// ===================================

void initSDCard()
{
    SerialUSB.println(F("\n=== SD Card Initialization ==="));
    SerialUSB.print(F("SD CS Pin: "));
    SerialUSB.println(SD_CS_PIN);

    // Try to initialize SD card
    if (!sdBrowser.begin(SD_CS_PIN))
    {
        SerialUSB.println(F("ERROR: SD Card initialization failed!"));
        SerialUSB.println(F("Possible issues:"));
        SerialUSB.println(F("  1. No SD card inserted"));
        SerialUSB.println(F("  2. Wrong CS pin (check SD_CS_PIN)"));
        SerialUSB.println(F("  3. SD card not formatted (use FAT16/FAT32)"));
        SerialUSB.println(F("  4. Wiring issues"));
        SerialUSB.println(F("  5. SD card corrupted"));

        // Show alert on screen
        ui_showAlert("SD Card Failed!", UI_ALERT_ERROR);
        return;
    }

    SerialUSB.println(F("SUCCESS: SD Card initialized!"));

    // Get file count
    int fileCount = sdBrowser.getFileCount();
    SerialUSB.print(F("Files found: "));
    SerialUSB.println(fileCount);

    // List all files
    if (fileCount > 0)
    {
        SerialUSB.println(F("\n--- Root Directory Contents ---"));
        for (int i = 0; i < fileCount; i++)
        {
            FileEntry *entry = sdBrowser.getFile(i);
            if (entry)
            {
                SerialUSB.print(i);
                SerialUSB.print(F(": "));
                SerialUSB.print(entry->name);
                if (entry->isDirectory)
                {
                    SerialUSB.println(F(" [DIR]"));
                }
                else
                {
                    SerialUSB.print(F(" ("));
                    SerialUSB.print(entry->size);
                    SerialUSB.println(F(" bytes)"));
                }
            }
        }
        SerialUSB.println(F("-------------------------------\n"));
    }
    else
    {
        SerialUSB.println(F("WARNING: SD card is empty or root directory has no files"));
    }

    SerialUSB.println(F("=== SD Card Initialization Complete ===\n"));
}

// ===================================
// Arduino Setup
// ===================================

void setup()
{
    // Initialize serial for debugging
    SerialUSB.begin(115200);
    delay(2000); // Wait for serial monitor
    SerialUSB.println(F("\n=== FarmBot UI Engine v3.0 ==="));

    // Initialize bare-metal hardware SPI (SERCOM1)
    SerialUSB.println(F("Initializing bare-metal SPI (SERCOM1)..."));
    spi_init();
    SerialUSB.println(F("SPI ready (12 MHz)!"));

    // Initialize TFT display (pins + ILI9341 init sequence)
    SerialUSB.println(F("Initializing TFT display..."));
    tft_init();
    SerialUSB.println(F("TFT display ready!"));

    // Touch controller shares SPI bus, CS pin configured by tft_init()
    SerialUSB.println(F("Touch controller ready (shared SPI bus)!"));

    // Initialize UI engine
    SerialUSB.println(F("Initializing UI engine..."));
    ui_init();

    // Set initial screen
    ui_setScreen(SCREEN_HOME);

    // Set initial status values
    ui_setBattery(75);
    ui_setGSM(80);
    ui_setGPS(false); // Will be updated by GPS module

    // Initialize SD Card with debugging
    initSDCard();

    // Initialize A9G GPS Module
    SerialUSB.println(F("Initializing A9G GPS..."));
    if (gpsModule.begin())
    {
        SerialUSB.println(F("GPS module initialized!"));
    }
    else
    {
        SerialUSB.println(F("GPS module failed to initialize"));
    }

    SerialUSB.println(F("=== System Ready ===\n"));
}

// ===================================
// Arduino Main Loop
// ===================================

void loop()
{
    // Main UI update (handles touch, screen drawing, etc.)
    ui_update();

    // Update sensor values periodically
    if (millis() - lastSensorUpdate > APP_SENSOR_UPDATE_INTERVAL)
    {
        lastSensorUpdate = millis();
        updateSensorValues();
    }

    // Update GPS data
    gpsModule.update();

    // Update GPS display every 3 seconds
    if (millis() - lastGPSUpdate > 3000)
    {
        lastGPSUpdate = millis();
        GPSData gpsData = gpsModule.getGPSData();
        ui_setGPS(gpsData.valid);
        ui_setGPSCoordinates(gpsData.latitude, gpsData.longitude, gpsData.valid);
    }

    // Small delay to prevent CPU hogging
    delay(1);
}