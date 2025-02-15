#include <SDL3/SDL.h>
#include <stdbool.h>

bool running = false;
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
Uint64 start_time = 0;
Uint64 current_time = 0;

#define FPS 60
#define TARGET_FRAME_TIME (1000 / FPS)

bool initialize(void) {
    if(!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        SDL_Log("ERROR: Could not initialize SDL: '%s'\n", SDL_GetError());
        return false;
    }
    if(!SDL_CreateWindowAndRenderer("Texture Easings", 1280, 720, 0, &window, &renderer)) {
        SDL_Log("ERROR: Could not create SDL window and renderer: '%s'\n", SDL_GetError());
        return false;
    }
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
    start_time = current_time;
}

void render(void) {
    SDL_SetRenderDrawColor(renderer, 0x18, 0x18, 0x18, 0xff);
    SDL_RenderClear(renderer);
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
