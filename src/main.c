#include <SDL3/SDL.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

#define FPS 60
#define TARGET_FRAME_TIME (1000 / FPS)
#define PENGER_SCALE_SIZE 64.0f
#define MAX_PENGERS 512 
#define SHINY_CHANCE 3

typedef enum {
    githubger,
    goldger,
    malger,
    michaelger,
    pengachu,
    penger,
    count
} penger_type;

typedef struct {
    float x;
    float y;
} vec2_t;

typedef struct {
    vec2_t pos;
    vec2_t vel;
    float size;
    float angle;
    bool shiny;
    SDL_Texture* texture;
} penger_t;



//////////////////////////
/// Global Variables
/////////////////////////
bool running = false;
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
Uint64 start_time = 0;
Uint64 current_time = 0;
float current_time_seconds = 0.0f;
penger_t pengers[MAX_PENGERS];
int screen_width = 1280;
int screen_height = 720;
int num_pengers = 0;
bool mouse_down = false;

//////////////////////////
/// Image Utilities
/////////////////////////
SDL_Texture* load_texture(const char* texture_path) {
    SDL_Surface* surface = SDL_LoadBMP(texture_path);
    if (surface == NULL) {
        SDL_Log("ERROR: Could not create surface for '%s' because of '%s'\n", texture_path, SDL_GetError());
        return NULL;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == NULL) {
        SDL_DestroySurface(surface);
        SDL_Log("ERROR: Could not create texture from surface because of '%s'\n", SDL_GetError());
        return NULL;
    }
    // Do not interpolate pixels
    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
    return texture;
}

//////////////////////////
/// Struct Creation
/////////////////////////
penger_t create_penger(float x, float y) {
    bool shiny = rand() % SHINY_CHANCE == (SHINY_CHANCE - 1);
    penger_type type_count = count;
    penger_type random_type = rand() % type_count;
    SDL_Texture* texture = NULL;
    switch (random_type) {
        case githubger:
            texture = load_texture("./assets/githubger.bmp");
            break;
        case goldger:
            texture = load_texture("./assets/goldger.bmp");
            break;
        case malger:
            texture = load_texture("./assets/malger.bmp");
            break;
        case michaelger:
            texture = load_texture("./assets/michaelger.bmp");
            break;
        case pengachu:
            texture = load_texture("./assets/pengachu.bmp");
            break;
        case penger:
            texture = load_texture("./assets/penger.bmp");
            break;
    }

    penger_t penger = {
        .size = PENGER_SCALE_SIZE,
        .pos = {
            .x = x,
            .y = y 
        },
        .vel = {
            .x = 4.0f,
            .y = 4.0f
        },
        .texture = texture,
        .shiny = shiny
    };
    return penger;
}

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
    return sinf((t * M_PI) / 2.0f);
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

    // Set VSync on in order to get rid of screen tearing.
    SDL_SetRenderVSync(renderer, 1);
    start_time = SDL_GetTicks();
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
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                mouse_down = true;
                break;
            case SDL_EVENT_MOUSE_BUTTON_UP:
                mouse_down = false;
                break;
        }
    }
}

void update_penger(penger_t* penger, float t, float vel_scale) {
        penger->angle = remap(t, 0.0f, 1.0f, 0.0f, 360.0f);
        penger->size = ease_out_sine(t) * PENGER_SCALE_SIZE;

        vec2_t new_pos = vec2_add(penger->pos, vec2_scale(penger->vel, vel_scale)); 
        if (new_pos.y <= 0 || (new_pos.y + penger->size)  > screen_height) {
            penger->vel = vec2_scale_y(penger->vel, -1.0f);
            return;
        }
        if (new_pos.x <= 0 || (new_pos.x + penger->size) > screen_width) {
            penger->vel = vec2_scale_x(penger->vel, -1.0f);
            return;
        }
        penger->pos = new_pos;
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
    float vel_scale = delta_time * 0.12f;

    if (mouse_down && num_pengers < MAX_PENGERS) {
        float mouse_x;
        float mouse_y;
        SDL_GetMouseState(&mouse_x, &mouse_y);
        penger_t penger = create_penger(mouse_x, mouse_y);
        pengers[num_pengers++] = penger;
    }
    for (int i = 0; i < num_pengers; i++) {
        penger_t* penger = &pengers[i];
        update_penger(penger, t, vel_scale);
    }
}


void render_penger(penger_t* penger) {
    SDL_FRect penger_rect = {
        .x = penger->pos.x,
        .y = penger->pos.y,
        .w = penger->size,
        .h = penger->size
    };
    if (penger->shiny) {
        // https://examples.libsdl.org/SDL3/renderer/11-color-mods/
        float r = (float) (0.5 + 0.5 * SDL_sin(current_time_seconds));
        float g = (float) (0.5 + 0.5 * SDL_sin(current_time_seconds + SDL_PI_D * 2 / 3));
        float b = (float) (0.5 + 0.5 * SDL_sin(current_time_seconds + SDL_PI_D * 4 / 3));
        SDL_SetTextureColorModFloat(penger->texture, r, g, b);
    }
    SDL_RenderTextureRotated(renderer, penger->texture, NULL, &penger_rect, penger->angle, NULL, 0);
}

void render(void) {
    SDL_SetRenderDrawColor(renderer, 0x18, 0x18, 0x18, 0xff);
    SDL_RenderClear(renderer);
    for (int i = 0; i < num_pengers; i++) {
        penger_t penger = pengers[i];
        render_penger(&penger);
    }
    SDL_RenderPresent(renderer);
}


void destroy(void) {
    for (int i = 0; i < num_pengers; i++) {
        SDL_DestroyTexture(pengers[i].texture);
    }
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
