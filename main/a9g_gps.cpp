/**
 * @file a9g_gps.cpp
 * @brief A9G GPS Module Implementation with Debug Support
 */

#include "a9g_gps.h"
#include <SD.h>

A9G_GPS::A9G_GPS()
{
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

    // Initialize debug info
    strcpy(debugInfo.lastCommand, "None");
    strcpy(debugInfo.lastResponse, "None");
    strcpy(debugInfo.gpsStatus, "Initializing...");
    strcpy(debugInfo.locationResponse, "No data");
    debugInfo.lastUpdateTime = 0;
    debugInfo.fixAttempts = 0;

    // Initialize NMEA buffer
    nmeaBuffer.writeIndex = 0;
    nmeaBuffer.count = 0;
    for (int i = 0; i < NMEA_BUFFER_SIZE; i++)
    {
        nmeaBuffer.sentences[i][0] = '\0';
    }
}

bool A9G_GPS::begin()
{
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

    if (!moduleOn)
    {
        SerialUSB.println(F("A9G: Module off, turning on..."));
        digitalWrite(A9G_PWR_KEY, LOW);
        delay(3000);
        digitalWrite(A9G_PWR_KEY, HIGH);
        delay(5000);
        moduleOn = checkModuleState();
    }

    if (moduleOn)
    {
        SerialUSB.println(F("A9G: Module ready!"));
        turnOnGPS();
        strcpy(debugInfo.gpsStatus, "Module Ready");
        return true;
    }
    else
    {
        SerialUSB.println(F("A9G: Failed to initialize!"));
        strcpy(debugInfo.gpsStatus, "Init Failed");
        return false;
    }
}

bool A9G_GPS::checkModuleState()
{
    for (int i = 0; i < 5; i++)
    {
        String response = sendCommand("AT", 1000);
        if (response.indexOf("OK") >= 0)
        {
            SerialUSB.println(F("A9G: Module responding"));
            return true;
        }
        delay(500);
    }
    return false;
}

void A9G_GPS::turnOnGPS()
{
    SerialUSB.println(F("A9G: Turning on GPS..."));

    // Turn on GPS
    String response = sendCommand("AT+GPS=1", 2000);
    SerialUSB.print(F("GPS ON Response: "));
    SerialUSB.println(response);

    // Check GPS status
    String status = sendCommand("AT+GPS?", 1000);
    SerialUSB.print(F("GPS Status: "));
    SerialUSB.println(status);

    // Enable NMEA output every 1 second for faster updates
    sendCommand("AT+GPSRD=1", 1000);

    SerialUSB.println(F("GPS initialization complete. Waiting for satellite fix..."));
    SerialUSB.println(F("Note: GPS may take 30-60 seconds for first fix. Needs clear sky view."));
}

void A9G_GPS::turnOffGPS()
{
    SerialUSB.println(F("A9G: Turning off GPS..."));
    sendCommand("AT+GPS=0", 2000);
}

void A9G_GPS::refreshDebugInfo()
{
    SerialUSB.println(F("Refreshing GPS..."));

    // Check GPS status first
    String status = sendCommand("AT+GPS?", 1000);
    SerialUSB.print(F("GPS Status: "));
    SerialUSB.println(status);

    // Get location
    String location = sendCommand("AT+LOCATION=2", 2000);
    SerialUSB.print(F("Location: "));
    SerialUSB.println(location);
}

void A9G_GPS::update()
{
    // Read GPS data periodically (simple approach like reference)
    if (millis() - lastGPSRead > gpsReadInterval)
    {
        // Get location - simple command
        String response = sendCommand("AT+LOCATION=2", 2000);

        // Simple parsing: just look for lat,lon pattern
        if (response.indexOf(",") > 0 && response.indexOf("OK") > 0)
        {
            // Got valid response with coordinates
            parseGPSLocation(response);
            logGPSData(); // Log to SD card
        }
        else
        {
            // No fix or error
            gpsData.valid = false;
        }

        lastGPSRead = millis();
    }

    // Process any incoming NMEA data for display only
    while (Serial1.available() > 0)
    {
        char c = Serial1.read();
        static String nmeaBuf = "";
        nmeaBuf += c;

        if (c == '\n')
        {
            // Just store NMEA sentences for display
            if (nmeaBuf.indexOf("$GP") >= 0 || nmeaBuf.indexOf("$GN") >= 0)
            {
                addNMEASentence(nmeaBuf.c_str());
            }
            nmeaBuf = "";
        }
    }
}

void A9G_GPS::parseGPSLocation(String response)
{
    // Simple parsing - just extract lat,lon from response
    // Response format: "lat,lon\n\nOK" or "+LOCATION: lat,lon,date,time"

    String locData = response;

    // Remove everything after "OK"
    int okIdx = locData.indexOf("OK");
    if (okIdx > 0)
    {
        locData = locData.substring(0, okIdx);
    }

    // If has "+LOCATION:", skip it
    int locIdx = locData.indexOf("+LOCATION:");
    if (locIdx >= 0)
    {
        locData = locData.substring(locIdx + 10);
    }

    // Clean up
    locData.trim();
    locData.replace("\r", "");
    locData.replace("\n", "");

    // Parse: lat,lon
    int comma = locData.indexOf(',');
    if (comma > 0)
    {
        gpsData.latitude = locData.substring(0, comma).toFloat();

        // Get longitude (may have more data after it)
        int comma2 = locData.indexOf(',', comma + 1);
        if (comma2 > 0)
        {
            gpsData.longitude = locData.substring(comma + 1, comma2).toFloat();
        }
        else
        {
            gpsData.longitude = locData.substring(comma + 1).toFloat();
        }

        // Mark as valid if coordinates are non-zero
        if (gpsData.latitude != 0.0 && gpsData.longitude != 0.0)
        {
            gpsData.valid = true;
            SerialUSB.print(F("GPS: "));
            SerialUSB.print(gpsData.latitude, 6);
            SerialUSB.print(F(", "));
            SerialUSB.println(gpsData.longitude, 6);
        }
        else
        {
            gpsData.valid = false;
        }
    }
}

String A9G_GPS::sendCommand(String cmd, unsigned long timeout)
{
    String response = "";
    Serial1.println(cmd);

    unsigned long start = millis();
    while (millis() - start < timeout)
    {
        while (Serial1.available())
        {
            char c = Serial1.read();
            response += c;
        }
    }

    return response;
}

GPSData A9G_GPS::getGPSData()
{
    return gpsData;
}

GPSDebugInfo A9G_GPS::getDebugInfo()
{
    return debugInfo;
}

NMEABuffer A9G_GPS::getNMEABuffer()
{
    return nmeaBuffer;
}

void A9G_GPS::addNMEASentence(const char *sentence)
{
    // Copy sentence to buffer (strip trailing whitespace)
    int len = strlen(sentence);
    if (len > 81)
        len = 81; // Limit to NMEA standard

    // Remove trailing \r\n
    while (len > 0 && (sentence[len - 1] == '\r' || sentence[len - 1] == '\n'))
    {
        len--;
    }

    strncpy(nmeaBuffer.sentences[nmeaBuffer.writeIndex], sentence, len);
    nmeaBuffer.sentences[nmeaBuffer.writeIndex][len] = '\0';

    // Update circular buffer indices
    nmeaBuffer.writeIndex = (nmeaBuffer.writeIndex + 1) % NMEA_BUFFER_SIZE;
    if (nmeaBuffer.count < NMEA_BUFFER_SIZE)
    {
        nmeaBuffer.count++;
    }
}

bool A9G_GPS::isGPSValid()
{
    return gpsData.valid;
}

String A9G_GPS::getLocationString()
{
    if (gpsData.valid)
    {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%.4f,%.4f", gpsData.latitude, gpsData.longitude);
        return String(buffer);
    }
    else
    {
        return "No GPS Fix";
    }
}

void A9G_GPS::logGPSData()
{
    if (!gpsData.valid)
        return;

    // Open GPS log file
    File logFile = SD.open("gps_log.txt", FILE_WRITE);
    if (logFile)
    {
        // Write timestamp and coordinates
        logFile.print(millis() / 1000);
        logFile.print(",");
        logFile.print(gpsData.latitude, 6);
        logFile.print(",");
        logFile.println(gpsData.longitude, 6);
        logFile.close();

        SerialUSB.println(F("GPS logged to SD"));
    }
}

String A9G_GPS::fetchLocationName()
{
    if (!gpsData.valid)
    {
        return "No GPS Fix";
    }

    SerialUSB.println(F("\n=== Fetching Location Name ==="));

    // Format the URL with coordinates
    char url[200];
    snprintf(url, sizeof(url),
             "aryan241.pythonanywhere.com/get-location?lat=%.6f&lon=%.6f",
             gpsData.latitude, gpsData.longitude);

    SerialUSB.print(F("URL: "));
    SerialUSB.println(url);

    // Setup HTTP connection
    String response = "";

    // Close any existing connections
    sendCommand("AT+HTTPTERM", 2000);
    delay(500);

    // Initialize HTTP service
    String result = sendCommand("AT+HTTPINIT", 3000);
    if (result.indexOf("OK") == -1)
    {
        SerialUSB.println(F("HTTP init failed"));
        return "HTTP Init Error";
    }
    delay(500);

    // Set HTTP parameters
    sendCommand("AT+HTTPPARA=\"CID\",1", 2000);

    // Set URL
    String urlCmd = "AT+HTTPPARA=\"URL\",\"" + String(url) + "\"";
    sendCommand(urlCmd, 2000);

    // Set content type
    sendCommand("AT+HTTPPARA=\"CONTENT\",\"application/json\"", 2000);

    // Perform HTTP GET request
    SerialUSB.println(F("Performing HTTP GET..."));
    result = sendCommand("AT+HTTPACTION=0", 15000); // 0 = GET, wait up to 15 seconds

    // Wait for response
    delay(2000);

    // Check HTTP status
    String statusCmd = sendCommand("AT+HTTPHEAD", 3000);
    SerialUSB.print(F("HTTP Header: "));
    SerialUSB.println(statusCmd);

    // Read HTTP response data
    result = sendCommand("AT+HTTPREAD", 5000);
    SerialUSB.print(F("HTTP Response: "));
    SerialUSB.println(result);

    // Parse JSON response to extract location
    String locationName = "Unknown";

    // Look for "location": "City, State" pattern
    int locIndex = result.indexOf("\"location\"");
    if (locIndex != -1)
    {
        int startQuote = result.indexOf("\"", locIndex + 12); // Skip past "location":
        if (startQuote != -1)
        {
            int endQuote = result.indexOf("\"", startQuote + 1);
            if (endQuote != -1)
            {
                locationName = result.substring(startQuote + 1, endQuote);
                SerialUSB.print(F("Extracted location: "));
                SerialUSB.println(locationName);

                // Save to cache for future use
                saveLocationCache(locationName);
            }
        }
    }

    // Terminate HTTP service
    sendCommand("AT+HTTPTERM", 2000);

    SerialUSB.println(F("=== Location Fetch Complete ===\n"));

    return locationName;
}

void A9G_GPS::saveLocationCache(String locationName)
{
    // Save location name to SD card cache file
    File cacheFile = SD.open("loc_cache.txt", FILE_WRITE);
    if (cacheFile)
    {
        cacheFile.seek(0); // Overwrite from beginning
        cacheFile.println(locationName);
        cacheFile.close();
        SerialUSB.println(F("Location cached to SD"));
    }
    else
    {
        SerialUSB.println(F("Failed to save location cache"));
    }
}

String A9G_GPS::loadLocationCache()
{
    String cachedLocation = "";

    if (!SD.exists("loc_cache.txt"))
    {
        SerialUSB.println(F("No location cache found - Press GET LOC to fetch location"));
        return "";
    }

    File cacheFile = SD.open("loc_cache.txt", FILE_READ);
    if (cacheFile)
    {
        if (cacheFile.available())
        {
            cachedLocation = cacheFile.readStringUntil('\n');
            cachedLocation.trim(); // Remove any trailing whitespace/newline
            SerialUSB.print(F("Loaded cached location: "));
            SerialUSB.println(cachedLocation);
        }
        else
        {
            SerialUSB.println(F("Cache file is empty"));
        }
        cacheFile.close();
    }
    else
    {
        SerialUSB.println(F("Failed to read location cache"));
    }

    return cachedLocation;
}