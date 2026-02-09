/**
 * @file a9g_gps.h
 * @brief A9G GPS Module Driver
 */

#ifndef A9G_GPS_H
#define A9G_GPS_H

#include <Arduino.h>

// A9G Module Control Pins
#define A9G_PWR_KEY 9
#define A9G_RST_KEY 6
#define A9G_LOW_PWR_KEY 5

// GPS Data Structure
typedef struct
{
    bool valid;
    float latitude;
    float longitude;
    float altitude;
    uint8_t satellites;
    char latDirection;   // 'N' or 'S'
    char lonDirection;   // 'E' or 'W'
    char lastUpdate[20]; // Time string
} GPSData;

// GPS Debug Info Structure
typedef struct
{
    char lastCommand[32];
    char lastResponse[256];
    char gpsStatus[64];
    char locationResponse[128];
    unsigned long lastUpdateTime;
    int fixAttempts;
} GPSDebugInfo;

// NMEA Circular Buffer
#define NMEA_BUFFER_SIZE 10
#define NMEA_MAX_LEN 83

typedef struct
{
    char sentences[NMEA_BUFFER_SIZE][NMEA_MAX_LEN];
    uint8_t writeIndex;
    uint8_t count;
} NMEABuffer;

class A9G_GPS
{
private:
    bool moduleOn;
    GPSData gpsData;
    GPSDebugInfo debugInfo;
    NMEABuffer nmeaBuffer;
    unsigned long lastGPSRead;
    unsigned long gpsReadInterval;

    String sendCommand(String cmd, unsigned long timeout);
    bool checkModuleState();
    void parseGPSLocation(String response);
    void parseNMEA(String nmea);
    void addNMEASentence(const char *sentence);
    void logGPSData();
    void saveLocationCache(String locationName);
    String loadLocationCache();

public:
    A9G_GPS();
    bool begin();
    void update();
    GPSData getGPSData();
    GPSDebugInfo getDebugInfo();
    NMEABuffer getNMEABuffer();
    bool isGPSValid();
    void turnOnGPS();
    void turnOffGPS();
    String getLocationString();
    void refreshDebugInfo();
    String fetchLocationName();
};

#endif // A9G_GPS_H