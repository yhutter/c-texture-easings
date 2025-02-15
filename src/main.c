#include <SDL3/SDL.h>
#include <stdbool.h>

typedef struct {
    float x;
    float y;
} vec2_t;

typedef struct {
    vec2_t pos;
    vec2_t vel;
    float size;
} player_t;

bool running = false;
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
Uint64 start_time = 0;
Uint64 current_time = 0;
player_t player = {0};
int screen_width = 1280;
int screen_height = 720;

#define FPS 60
#define TARGET_FRAME_TIME (1000 / FPS)

vec2_t vec2_add(vec2_t a, vec2_t b) {
    vec2_t result = {
        .x = a.x + b.x,
        .y = a.y + b.y
    };
    return result;
}

vec2_t vec2_scale(vec2_t a, float scale) {
    vec2_t result = {
        .x = a.x * scale,
        .y = a.y * scale 
    };
    return result;
}

bool initialize(void) {
    if(!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        SDL_Log("ERROR: Could not initialize SDL: '%s'\n", SDL_GetError());
        return false;
    }
    if(!SDL_CreateWindowAndRenderer("Texture Easings", screen_width, screen_height, 0, &window, &renderer)) {
        SDL_Log("ERROR: Could not create SDL window and renderer: '%s'\n", SDL_GetError());
        return false;
    }
    start_time = SDL_GetTicks();

    player.pos = (vec2_t) {
        .x = screen_width / 2.0f,
        .y = screen_height / 2.0f
    };
    player.size = 20.0f;
    player.vel = (vec2_t) {
        .x = 0.0f,
        .y = 5.0f
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
    int time_to_wait = TARGET_FRAME_TIME - delta_time;
    if (time_to_wait > 0 && time_to_wait <= TARGET_FRAME_TIME) {
        SDL_Delay(time_to_wait);
    }

    player.pos = vec2_add(player.pos, vec2_scale(player.vel, delta_time * 0.15f));

    if (player.pos.y > screen_height) {
        player.pos.y = 0;
    }
    start_time = current_time;
}

void render(void) {
    SDL_SetRenderDrawColor(renderer, 0x18, 0x18, 0x18, 0xff);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 0xff, 0xdd, 0x33, 0xff);
    SDL_FRect player_rect = {
        .x = player.pos.x,
        .y = player.pos.y,
        .w = player.size,
        .h = player.size
    };
    SDL_RenderFillRect(renderer, &player_rect);
    SDL_RenderPresent(renderer);
}

void destroy(void) {
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
