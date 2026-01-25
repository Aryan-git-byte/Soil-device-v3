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
#include "file_browser.h"  // ← This line MUST be here!

// ===================================
// Application Configuration
// ===================================

// Sensor update interval in milliseconds
#define APP_SENSOR_UPDATE_INTERVAL 2000

// ===================================
// Application State
// ===================================
FileBrowser sdBrowser;  // <-- Make sure this line exists!

static uint32_t lastSensorUpdate = 0;

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
// Arduino Setup
// ===================================

void setup()
{
    // Initialize serial for debugging
    SerialUSB.begin(115200);
    delay(2000); // Wait for serial monitor
    SerialUSB.println(F("\n=== FarmBot UI Engine v3.0 ==="));

    // Initialize TFT display pins and controller
    SerialUSB.println(F("Initializing TFT display..."));
    tft_initPins();
    tft_init();
    SerialUSB.println(F("TFT display ready!"));

    // Initialize touch controller pins
    SerialUSB.println(F("Initializing touch controller..."));
    touch_initPins();
    SerialUSB.println(F("Touch controller ready!"));

    // Initialize UI engine
    SerialUSB.println(F("Initializing UI engine..."));
    ui_init();

    // Set initial screen
    ui_setScreen(SCREEN_HOME);

    // Set initial status values
    ui_setBattery(75);
    ui_setGSM(80);
    ui_setGPS(true);

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

    // Small delay to prevent CPU hogging
    delay(1);
}
