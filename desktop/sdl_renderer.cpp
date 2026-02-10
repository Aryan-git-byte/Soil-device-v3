#include <Arduino.h>
#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdio.h> // Added for debug prints

static SDL_Window* window = nullptr;
static SDL_Renderer* renderer = nullptr;

static const int W = 240;
static const int H = 320;

// Framebuffer (RGB888)
static uint32_t framebuffer[W * H];

// State for mouse/touch
static bool is_mouse_down = false;
static int mouse_x = 0;
static int mouse_y = 0;


// ================= INIT =================
void sdl_init()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL Init Failed: %s\n", SDL_GetError());
        return;
    }

    window = SDL_CreateWindow(
        "SoilDevice Emulator",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        W * 2, // 2x Scale for easier clicking
        H * 2,
        SDL_WINDOW_SHOWN
    );

    if (!window) {
        printf("Window Init Failed: %s\n", SDL_GetError());
        return;
    }

    renderer = SDL_CreateRenderer(window, -1,
                                  SDL_RENDERER_ACCELERATED |
                                  SDL_RENDERER_PRESENTVSYNC);
}


// ================= HELPERS =================
static inline uint32_t rgb565_to_888(uint16_t c)
{
    uint8_t r = ((c >> 11) & 0x1F) << 3;
    uint8_t g = ((c >> 5) & 0x3F) << 2;
    uint8_t b = (c & 0x1F) << 3;

    return (255 << 24) | (r << 16) | (g << 8) | b;
}


// ================= DRAW OPS =================
void sdl_clear(uint16_t color565)
{
    uint32_t col = rgb565_to_888(color565);
    for(int i=0;i<W*H;i++)
        framebuffer[i] = col;
}


void sdl_drawPixel(int x, int y, uint16_t color565)
{
    if(x < 0 || x >= W || y < 0 || y >= H)
        return;

    framebuffer[y * W + x] = rgb565_to_888(color565);
}


void sdl_present()
{
    SDL_Texture* tex = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        W, H
    );

    SDL_UpdateTexture(tex, nullptr, framebuffer, W * 4);

    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, tex, nullptr, nullptr);
    SDL_RenderPresent(renderer);

    SDL_DestroyTexture(tex);
}

// ---- TOUCH BRIDGE (FIXED) ----

bool sdl_touch(int16_t* x, int16_t* y)
{
    SDL_Event e;

    // Process all pending events
    while (SDL_PollEvent(&e))
    {
        if (e.type == SDL_QUIT) {
            exit(0);
        }
        else if (e.type == SDL_MOUSEBUTTONDOWN) {
            is_mouse_down = true;
            mouse_x = e.button.x / 2; // Divide by 2 because window is 2x scale
            mouse_y = e.button.y / 2;
        }
        else if (e.type == SDL_MOUSEBUTTONUP) {
            is_mouse_down = false;
        }
        else if (e.type == SDL_MOUSEMOTION) {
            if (is_mouse_down) {
                mouse_x = e.motion.x / 2;
                mouse_y = e.motion.y / 2;
            }
        }
    }

    if (is_mouse_down) {
        *x = (int16_t)mouse_x;
        *y = (int16_t)mouse_y;
        
        // Safety clamp
        if(*x < 0) *x = 0;
        if(*y < 0) *y = 0;
        if(*x >= W) *x = W-1;
        if(*y >= H) *y = H-1;

        return true;
    }

    return false;
}