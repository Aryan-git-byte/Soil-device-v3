#include <Arduino.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "../main/file_browser.h"
#include "../main/a9g_gps.h"
#include "../main/tft_driver.h"

// SDL bridge functions
extern void sdl_init();
extern void sdl_clear(uint16_t);
extern void sdl_present();
extern void sdl_drawPixel(int,int,uint16_t);
extern bool sdl_touch(int16_t*,int16_t*);


// ------------------------------------------------
// GLOBAL OBJECTS EXPECTED BY UI
// ------------------------------------------------

FileBrowser sdBrowser;
A9G_GPS gpsModule;


// ------------------------------------------------
// TOUCH
// ------------------------------------------------

bool touch_getPoint(int16_t* x,int16_t* y)
{
    return sdl_touch(x,y);
}


// ------------------------------------------------
// TFT DRIVER STUB IMPLEMENTATION
// ------------------------------------------------

// Track the current window bounds and cursor position
static uint16_t _x0, _y0, _x1, _y1;
static uint16_t _cursorX, _cursorY;

void tft_init(void)
{
    sdl_init();
}

void tft_setWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    // Save bounds
    _x0 = x0; _y0 = y0;
    _x1 = x1; _y1 = y1;
    // Reset cursor to start
    _cursorX = x0;
    _cursorY = y0;
}

void tft_beginWrite(){}
void tft_endWrite(){}

// Helper to draw and move cursor (Simulate hardware auto-increment)
void write_pixel_auto_move(uint16_t color) {
    sdl_drawPixel(_cursorX, _cursorY, color);

    _cursorX++;
    if(_cursorX > _x1) {
        _cursorX = _x0;
        _cursorY++;
    }
}

void tft_writeColor(uint16_t color)
{
    write_pixel_auto_move(color);
}

void tft_writeData16(uint16_t color)
{
    write_pixel_auto_move(color);
}


// ------------------------------------------------
// FILE BROWSER STUB
// ------------------------------------------------

FileBrowser::FileBrowser(){
    // Initialize the member variables that the INLINE getters return
    fileCount = 5;
    scrollOffset = 0;
    selectedIndex = -1;
    currentPath = "/";
}

bool FileBrowser::begin(uint8_t){ 
    fileCount = 5;
    return true; 
}

void FileBrowser::openDirectory(const char* path){
    currentPath = String(path);
    // Reset fake files on directory change
    fileCount = 5;
    scrollOffset = 0;
    selectedIndex = -1;
}

FileEntry* FileBrowser::getFile(int i)
{
    // Access the private 'files' array directly since we are in the class scope
    if (i < 0 || i >= MAX_FILES_DISPLAY) return nullptr;

    // Generate some fake filenames based on index
    if (i == 0) strcpy(files[i].name, "data_log.csv");
    else if (i == 1) strcpy(files[i].name, "config.ini");
    else if (i == 2) strcpy(files[i].name, "images"); // Directory
    else sprintf(files[i].name, "record_%02d.txt", i);

    files[i].isDirectory = (i == 2); // Make index 2 a folder
    files[i].size = 1024 * (i + 1);

    return &files[i];
}

void FileBrowser::scroll(int delta){
    scrollOffset += delta;
    if(scrollOffset < 0) scrollOffset = 0;
    if(scrollOffset > fileCount - 1) scrollOffset = fileCount - 1;
}

void FileBrowser::goUp(){
    if(currentPath.length() > 1) {
        currentPath = "/";
        openDirectory("/");
    }
}

void FileBrowser::selectFile(int i){
    selectedIndex = i;
}


// ------------------------------------------------
// GPS STUB
// ------------------------------------------------

A9G_GPS::A9G_GPS(){}

bool A9G_GPS::begin(){ return true; }

void A9G_GPS::update(){} 

GPSData A9G_GPS::getGPSData()
{
    GPSData d{};
    d.valid = true;
    d.latitude = 28.6139; // New Delhi Lat
    d.longitude = 77.2090; // New Delhi Lon
    d.satellites = 5;
    d.altitude = 210.5;
    return d;
}

GPSDebugInfo A9G_GPS::getDebugInfo()
{
    GPSDebugInfo i{};
    strcpy(i.lastCommand, "AT+GPS=1");
    strcpy(i.gpsStatus, "3D Fix");
    strcpy(i.locationResponse, "28.6139,77.2090");
    i.fixAttempts = 1;
    i.lastUpdateTime = 12345;
    return i;
}

void A9G_GPS::refreshDebugInfo(){}
void A9G_GPS::turnOnGPS(){}
void A9G_GPS::turnOffGPS(){}
// Removed the invalid setGPS functions