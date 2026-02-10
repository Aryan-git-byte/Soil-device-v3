#include <stdio.h>
#include <stdint.h>
#include <SDL2/SDL.h>

// Pull firmware in
#include "../main/ui_engine.cpp"
#include "../main/screens.cpp"
#include "../main/drawing.cpp"
#include "../main/tft_driver.h"

// SDL hooks
extern void sdl_init();
extern void sdl_present(); // Fix: This was missing
extern bool touch_getPoint(int16_t*, int16_t*);

int main()
{
    printf("SIM STARTED\n");

    // Init virtual display
    tft_init();

    // Init firmware UI system
    ui_init();

    // ⭐ IMPORTANT — force first screen
    ui_setScreen(SCREEN_HOME);

    // Draw once
    ui_drawScreen();

    // ---------- Loop ----------
    while(true)
    {
        int16_t x,y;

        // Handle Touch
        if(touch_getPoint(&x,&y))
            ui_handleTouch(x,y);

        // Update UI Logic
        ui_update();

        // Fix: Push the drawing to the window!
        sdl_present();

        // Limit FPS to ~60
        SDL_Delay(16);
    }

    return 0;
}