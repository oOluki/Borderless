#include "renderer.h"
#include <SDL2/SDL.h>

typedef struct Game Game;

typedef struct Game{

    Surface draw_canvas;

    void* user_data;

    int (*update)(Game*);
    int (*draw)(Game*);
    int (*handle_events)(Game*);
} Game;


int handle_events(const SDL_Event event){
    switch (event.type)
    {
    case SDL_QUIT:
        return 1;
    
    default:
        return 0;
    }
}



int main(int argc, char** argv){

    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window* window = SDL_CreateWindow("Boundless", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_RESIZABLE);

    SDL_Event events;

    while(1){

        while(SDL_PollEvent(&events))
            if(handle_events(events))
                break;

    }


    SDL_Quit();

    return 0;
}
