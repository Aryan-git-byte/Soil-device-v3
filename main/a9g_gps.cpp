/**
 * @file a9g_gps.cpp
 * @brief A9G GPS Module Implementation
 */

#include "a9g_gps.h"

A9G_GPS::A9G_GPS() {
    moduleOn = false;
    gpsData.valid = false;
    gpsData.latitude = 0.0;
    gpsData.longitude = 0.0;
    gpsData.altitude = 0.0;
    gpsData.satellites = 0;
    gpsData.latDirection = 'N';
    gpsData.lonDirection = 'E';
    strcpy(gpsData.lastUpdate, "No Fix");
    lastGPSRead = 0;
    gpsReadInterval = 5000; // Read GPS every 5 seconds
}

bool A9G_GPS::begin() {
    // Setup control pins
    pinMode(A9G_PWR_KEY, OUTPUT);
    pinMode(A9G_RST_KEY, OUTPUT);
    pinMode(A9G_LOW_PWR_KEY, OUTPUT);
    
    digitalWrite(A9G_RST_KEY, LOW);
    digitalWrite(A9G_LOW_PWR_KEY, HIGH);
    digitalWrite(A9G_PWR_KEY, HIGH);
    
    // Initialize Serial1 for A9G communication
    Serial1.begin(115200);
    
    SerialUSB.println(F("A9G: Initializing module..."));
    
    // Power on sequence
    digitalWrite(A9G_PWR_KEY, LOW);
    delay(3000);
    digitalWrite(A9G_PWR_KEY, HIGH);
    delay(5000);
    
    // Check if module is on
    moduleOn = checkModuleState();
    
    if (!moduleOn) {
        SerialUSB.println(F("A9G: Module off, turning on..."));
        digitalWrite(A9G_PWR_KEY, LOW);
        delay(3000);
        digitalWrite(A9G_PWR_KEY, HIGH);
        delay(5000);
        moduleOn = checkModuleState();
    }
    
    if (moduleOn) {
        SerialUSB.println(F("A9G: Module ready!"));
        turnOnGPS();
        return true;
    } else {
        SerialUSB.println(F("A9G: Failed to initialize!"));
        return false;
    }
}

bool A9G_GPS::checkModuleState() {
    for (int i = 0; i < 5; i++) {
        String response = sendCommand("AT", 1000);
        if (response.indexOf("OK") >= 0) {
            SerialUSB.println(F("A9G: Module responding"));
            return true;
        }
        delay(500);
    }
    return false;
}

void A9G_GPS::turnOnGPS() {
    SerialUSB.println(F("A9G: Turning on GPS..."));
    sendCommand("AT+GPS=1", 2000);
    sendCommand("AT+GPSRD=10", 1000); // Read NMEA every 10 seconds
}

void A9G_GPS::turnOffGPS() {
    SerialUSB.println(F("A9G: Turning off GPS..."));
    sendCommand("AT+GPS=0", 2000);
}

void A9G_GPS::update() {
    // Read GPS data periodically
    if (millis() - lastGPSRead > gpsReadInterval) {
        String response = sendCommand("AT+LOCATION=2", 2000);
        parseGPSLocation(response);
        lastGPSRead = millis();
    }
    
    // Process any incoming NMEA data
    while (Serial1.available() > 0) {
        char c = Serial1.read();
        static String nmeaBuffer = "";
        nmeaBuffer += c;
        
        if (c == '\n') {
            if (nmeaBuffer.indexOf("$GP") >= 0) {
                parseNMEA(nmeaBuffer);
            }
            nmeaBuffer = "";
        }
    }
}

void A9G_GPS::parseGPSLocation(String response) {
    // Parse response like: +LOCATION: 25.123456,85.234567,2024/01/25,12:30:45
    int locIndex = response.indexOf("+LOCATION:");
    if (locIndex >= 0) {
        String locData = response.substring(locIndex + 10);
        locData.trim();
        
        // Parse latitude
        int comma1 = locData.indexOf(',');
        if (comma1 > 0) {
            String lat = locData.substring(0, comma1);
            gpsData.latitude = lat.toFloat();
            
            // Parse longitude
            int comma2 = locData.indexOf(',', comma1 + 1);
            if (comma2 > 0) {
                String lon = locData.substring(comma1 + 1, comma2);
                gpsData.longitude = lon.toFloat();
                
                // Parse date/time
                int comma3 = locData.indexOf(',', comma2 + 1);
                if (comma3 > 0) {
                    String dateTime = locData.substring(comma2 + 1);
                    dateTime.trim();
                    strncpy(gpsData.lastUpdate, dateTime.c_str(), 19);
                    gpsData.lastUpdate[19] = '\0';
                }
                
                gpsData.valid = true;
                
                SerialUSB.print(F("GPS: "));
                SerialUSB.print(gpsData.latitude, 6);
                SerialUSB.print(F(", "));
                SerialUSB.println(gpsData.longitude, 6);
            }
        }
    }
}

void A9G_GPS::parseNMEA(String nmea) {
    // Parse GPGGA for satellite count and altitude
    if (nmea.indexOf("$GPGGA") >= 0) {
        // Simple parsing - in production use a proper NMEA parser
        int commaCount = 0;
        int lastComma = 0;
        for (int i = 0; i < nmea.length(); i++) {
            if (nmea[i] == ',') {
                commaCount++;
                if (commaCount == 7) { // Satellite count is 7th field
                    String satStr = nmea.substring(lastComma + 1, i);
                    gpsData.satellites = satStr.toInt();
                }
                if (commaCount == 9) { // Altitude is 9th field
                    String altStr = nmea.substring(lastComma + 1, i);
                    gpsData.altitude = altStr.toFloat();
                }
                lastComma = i;
            }
        }
    }
}

String A9G_GPS::sendCommand(String cmd, unsigned long timeout) {
    String response = "";
    Serial1.println(cmd);
    
    unsigned long start = millis();
    while (millis() - start < timeout) {
        while (Serial1.available()) {
            char c = Serial1.read();
            response += c;
        }
    }
    
    return response;
}

GPSData A9G_GPS::getGPSData() {
    return gpsData;
}

bool A9G_GPS::isGPSValid() {
    return gpsData.valid;
}

String A9G_GPS::getLocationString() {
    if (gpsData.valid) {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%.4f,%.4f", gpsData.latitude, gpsData.longitude);
        return String(buffer);
    } else {
        return "No GPS Fix";
    }
}