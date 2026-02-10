#ifndef SCREENS_H
#define SCREENS_H

#include "pages/home/home_page.h"
#include "pages/files/files_page.h"
// #include "pages/gps/gps_page.h"  <-- Do this for GPS too

// Keep the others here until you move them
void screen_ai_draw(void);
void screen_settings_draw(void);
void screen_input_draw(void);
void screen_gps_debug_draw(void);
void screen_gps_debug_handleTouch(int16_t x, int16_t y);
void screen_gps_debug_update(void);

#endif