#include <SDL3/SDL.h>
#include <stdbool.h>
#include <math.h>

typedef struct {
    float x;
    float y;
} vec2_t;

typedef struct {
    vec2_t pos;
    vec2_t vel;
    float size;
} penger_t;

//////////////////////////
/// Global Variables
/////////////////////////
bool running = false;
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Texture* texture = NULL;
SDL_Surface* surface = NULL;
Uint64 start_time = 0;
Uint64 current_time = 0;
float current_time_seconds = 0.0f;
penger_t penger = {0};
int screen_width = 1280;
int screen_height = 720;

#define FPS 60
#define TARGET_FRAME_TIME (1000 / FPS)
#define SCALE_SIZE 128.0f

//////////////////////////
/// Vector Utilities
/////////////////////////
static inline vec2_t vec2_add(vec2_t a, vec2_t b) {
    vec2_t result = {
        .x = a.x + b.x,
        .y = a.y + b.y
    };
    return result;
}

static inline vec2_t vec2_scale(vec2_t a, float scale) {
    vec2_t result = {
        .x = a.x * scale,
        .y = a.y * scale 
    };
    return result;
}

static inline vec2_t vec2_scale_y(vec2_t a, float scale) {
    vec2_t result = {
        .x = a.x,
        .y = a.y * scale 
    };
    return result;
}

static inline vec2_t vec2_scale_x(vec2_t a, float scale) {
    vec2_t result = {
        .x = a.x * scale,
        .y = a.y 
    };
    return result;
}

//////////////////////////
/// Math Utilities
/////////////////////////
static inline float remap(float value, float in_min, float in_max, float out_min, float out_max) {
    float in_range = in_max - in_min;
    float out_range = out_max - out_min;
    float percentage = (value - in_min) / in_range;
    return out_min + (out_range * percentage);
}


//////////////////////////
/// Easing Functions
/////////////////////////
static inline float ease_in_sine(float t) {
    return 1.0f - cosf((t * M_PI) / 2.0f);
}

static inline float ease_out_sine(float t) {
    return sinf((t * M_PI) / 2.0f);;
}

//////////////////////////
/// Game Loop Functions
/////////////////////////
bool initialize(void) {
    if(!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        SDL_Log("ERROR: Could not initialize SDL: '%s'\n", SDL_GetError());
        return false;
    }
    if(!SDL_CreateWindowAndRenderer("Texture Easings", screen_width, screen_height, 0, &window, &renderer)) {
        SDL_Log("ERROR: Could not create SDL window and renderer: '%s'\n", SDL_GetError());
        return false;
    } 

    const char* texture_path = "./assets/penger.bmp";
    surface = SDL_LoadBMP(texture_path);
    if (surface == NULL) {
        SDL_Log("ERROR: Could not surface '%s'\n", texture_path);
        return false;
    }

    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == NULL) {
        SDL_Log("ERROR: Could not create texture from surface '%s'\n", SDL_GetError());
        return false;
    }
    
    // Do not interpolate pixels
    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);

    // Set VSync on in order to get rid of screen tearing.
    SDL_SetRenderVSync(renderer, 1);
    start_time = SDL_GetTicks();

    penger = (penger_t) {
        .size = SCALE_SIZE,
        .pos = {
            .x = screen_width / 2.0f,
            .y = screen_height / 2.0f
        },
        .vel = {
            .x = 4.0f,
            .y = 4.0f
        }
    };
    return true;
}

void process(void) {
    SDL_Event e;
    while(SDL_PollEvent(&e)) {
        switch(e.type) {
            case SDL_EVENT_QUIT:
                running = false;
                break;
            case SDL_EVENT_KEY_DOWN:
                if(e.key.key == SDLK_ESCAPE) {
                    running = false;
                    break;
                }
        }
    }
}

void update(void) {
    current_time = SDL_GetTicks();
    Uint64 delta_time = current_time - start_time;
    current_time_seconds += delta_time * 0.001f;
    int time_to_wait = TARGET_FRAME_TIME - delta_time;
    if (time_to_wait > 0 && time_to_wait <= TARGET_FRAME_TIME) {
        SDL_Delay(time_to_wait);
    }
    start_time = current_time;

    float t  = remap(sinf(current_time_seconds), -1.0f, 1.0f, 0.0f, 1.0f);
    penger.size = ease_out_sine(t) * SCALE_SIZE;

    float vel_scale = delta_time * 0.12f;
    vec2_t new_pos = vec2_add(penger.pos, vec2_scale(penger.vel, vel_scale)); 
    if (new_pos.y <= 0 || (new_pos.y + penger.size)  > screen_height) {
        penger.vel = vec2_scale_y(penger.vel, -1.0f);
        return;
    }
    if (new_pos.x <= 0 || (new_pos.x + penger.size) > screen_width) {
        penger.vel = vec2_scale_x(penger.vel, -1.0f);
        return;
    }
    penger.pos = new_pos;
}

void render_penger(penger_t* penger) {
    float t = remap(sinf(current_time_seconds), -1.0f, 1.0, 0.0f, 1.0f);
    float angle = remap(t, 0.0f, 1.0f, 0.0f, 360.0f);

    SDL_FRect penger_rect = {
        .x = penger->pos.x,
        .y = penger->pos.y,
        .w = penger->size,
        .h = penger->size
    };
    SDL_SetTextureColorModFloat(texture, t, t, t);
    // SDL_RenderTexture(renderer, texture, NULL, &penger_rect);
    SDL_RenderTextureRotated(renderer, texture, NULL, &penger_rect, angle, NULL, 0);
}

void render(void) {
    SDL_SetRenderDrawColor(renderer, 0x18, 0x18, 0x18, 0xff);
    SDL_RenderClear(renderer);
    render_penger(&penger);
    SDL_RenderPresent(renderer);
}


void destroy(void) {
    SDL_DestroyTexture(texture);
    SDL_DestroySurface(surface);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int main(void) {
    running = initialize();
    while(running) {
        process();
        update();
        render();
    }
    destroy();
    return 0;
}
