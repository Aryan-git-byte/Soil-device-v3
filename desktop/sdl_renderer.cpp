#include <SDL2/SDL.h>
#include <stdint.h>

static SDL_Window* window = nullptr;
static SDL_Renderer* renderer = nullptr;

static const int W = 240;
static const int H = 320;

// Framebuffer (RGB888)
static uint32_t framebuffer[W * H];


// ================= INIT =================
void sdl_init()
{
    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow(
        "SoilDevice Emulator",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        W * 2,
        H * 2,
        SDL_WINDOW_SHOWN
    );

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

// ---- TOUCH BRIDGE ----
#include <SDL2/SDL.h>

bool sdl_touch(int16_t* x, int16_t* y)
{
    SDL_Event e;

    while (SDL_PollEvent(&e))
    {
        if (e.type == SDL_QUIT)
            exit(0);

        if (e.type == SDL_MOUSEBUTTONDOWN)
        {
            *x = e.button.x;
            *y = e.button.y;
            return true;
        }
    }
    return false;
}
