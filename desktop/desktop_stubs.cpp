#include <stdint.h>
#include <string.h>

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

static uint16_t winX,winY;

void tft_init(void)
{
    sdl_init();
}

void tft_setWindow(uint16_t x0,uint16_t y0,uint16_t,uint16_t)
{
    winX = x0;
    winY = y0;
}

void tft_beginWrite(){}
void tft_endWrite(){}

void tft_writeColor(uint16_t color)
{
    sdl_drawPixel(winX,winY,color);
}

void tft_writeData16(uint16_t color)
{
    sdl_drawPixel(winX,winY,color);
}


// ------------------------------------------------
// FILE BROWSER STUB
// ------------------------------------------------

static FileEntry fakeFile;

FileBrowser::FileBrowser(){}

FileEntry* FileBrowser::getFile(int)
{
    strcpy(fakeFile.name,"mock.txt");
    fakeFile.isDirectory=false;
    return &fakeFile;
}

void FileBrowser::scroll(int){}
void FileBrowser::goUp(){}
void FileBrowser::selectFile(int){}


// ------------------------------------------------
// GPS STUB
// ------------------------------------------------

A9G_GPS::A9G_GPS(){}

GPSData A9G_GPS::getGPSData()
{
    GPSData d{};
    d.latitude=28.6;
    d.longitude=77.2;
    d.valid=true;
    return d;
}

GPSDebugInfo A9G_GPS::getDebugInfo()
{
    GPSDebugInfo i{};
    return i;
}

void A9G_GPS::refreshDebugInfo(){}
void A9G_GPS::turnOnGPS(){}
void A9G_GPS::turnOffGPS(){}

