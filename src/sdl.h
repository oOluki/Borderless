#ifndef BSDL_HEADER
#define BSDL_HEADER

#include "game.h"
#include <SDL2/SDL.h>

typedef struct UserData{

    SDL_Window*     window;
    SDL_Surface*    window_surface;
    SDL_Surface*    framebuffer;
    SDL_Event       event;
    Uint64          t;

    SDL_Rect        mouse;

    int             continuos;
    Uint64          chill;

} UserData;

void handle_keyup(Game* game, SDL_Keycode key){
    UserData* const user_data = (UserData*) game->user_data;
    switch (key)
    {
    case SDLK_w:
        if(!user_data->continuos && 0 == get_tile(game->map, game->player.x / TILEW, game->player.y / TILEH - 1)){
            game->player.y -= TILEH;
            if(game->update) game->update(game);
        }
        break;
    case SDLK_a:
        if(!user_data->continuos && 0 == get_tile(game->map, game->player.x / TILEW - 1, game->player.y / TILEH)){
            game->player.x -= TILEW;
            if(game->update) game->update(game);
        }
        break;
    case SDLK_s:
        if(!user_data->continuos && 0 == get_tile(game->map, game->player.x / TILEW, game->player.y / TILEH + 1)){
            game->player.y += TILEH;
            if(game->update) game->update(game);
        }
        break;
    case SDLK_d:
        if(!user_data->continuos && 0 == get_tile(game->map, game->player.x / TILEW + 1, game->player.y / TILEH)){
            game->player.x += TILEW;
            if(game->update) game->update(game);
        }
        break;
    
    case SDLK_LSHIFT:
    case SDLK_RSHIFT:
        user_data->continuos = 0;
        break;
    case SDLK_RCTRL:
    case SDLK_LCTRL:
        game->debug = 1;
        break;
    
    default:
        break;
    }
}

void handle_keydown(Game* game, SDL_Keycode key){
    UserData* const user_data = (UserData*) game->user_data;
    switch (key)
    {
    case SDLK_w:
        if(user_data->continuos && user_data->chill > 60 * 16 && 0 == get_tile(game->map, game->player.x / TILEW, game->player.y / TILEH - 1)){
            game->player.y -= TILEH;
            if(game->update) game->update(game);
        }
        break;
    case SDLK_a:
        if(user_data->continuos && user_data->chill > 60 * 16 && 0 == get_tile(game->map, game->player.x / TILEW - 1, game->player.y / TILEH)){
            game->player.x -= TILEW;
            if(game->update) game->update(game);
        }
        break;
    case SDLK_s:
        if(user_data->continuos && user_data->chill > 60 * 16 && 0 == get_tile(game->map, game->player.x / TILEW, game->player.y / TILEH + 1)){
            game->player.y += TILEH;
            if(game->update) game->update(game);
        }
        break;
    case SDLK_d:
        if(user_data->continuos && user_data->chill > 60 * 16 && 0 == get_tile(game->map, game->player.x / TILEW + 1, game->player.y / TILEH)){
            game->player.x += TILEW;
            if(game->update) game->update(game);
        }
        break;
    
    case SDLK_RSHIFT:
    case SDLK_LSHIFT:
        user_data->continuos = 1;
        break;
    
    default:
        break;
    }
}

int user_update(Game* game){
    UserData* const user_data = (UserData*) game->user_data;
    SDL_Event* const event = &user_data->event;
    while(SDL_PollEvent(event)) switch (event->type)
    {
    case SDL_QUIT:
        game->active = 0;
        return 0;
    case SDL_KEYUP:
        handle_keyup(game, event->key.keysym.sym);
        break;
    case SDL_KEYDOWN:
        handle_keydown(game, event->key.keysym.sym);
        break;
    case SDL_MOUSEMOTION:
        game->mouse.x = (event->motion.x * game->camera.w) / user_data->window_surface->w;
        game->mouse.y = (event->motion.y * game->camera.h) / user_data->window_surface->h;
        break;
    case SDL_MOUSEBUTTONDOWN:
        printf(
            "mouse to player: %f\n",
            sqrt(distance2(
                game->mouse.x, game->mouse.y, game->player.x - game->camera.x, game->player.y - game->camera.y
            )));
        break;
    case SDL_WINDOWEVENT:
        user_data->window_surface = SDL_GetWindowSurface(user_data->window);
        break;
    
    default:
        break;
    }

    { // clock
        const Uint64 t1 = SDL_GetTicks64();
        if((t1 - user_data->t) < (1000 / FPS)){
            SDL_Delay((Uint32)((1000 / FPS) + user_data->t - t1));
            user_data->chill += (1000 / FPS);
        } else{
            user_data->chill += (t1 - user_data->t);
        }
        user_data->t = SDL_GetTicks64();
    }

    return 0;
}

int user_draw(Game* game){

    UserData* const user_data = (UserData*) game->user_data;

    const SDL_Rect src = (SDL_Rect){
        .x = 0,
        .y = 0,
        .w = game->camera.w,
        .h = game->camera.h
    };

    SDL_Rect dest = (SDL_Rect){
        .x = 0, .y = 0,
        .w = user_data->window_surface->w,
        .h = user_data->window_surface->h
    };

    SDL_BlitScaled(user_data->framebuffer, &src, user_data->window_surface, &dest);

    SDL_UpdateWindowSurface(user_data->window);
    
    return 0;
}

int init_subsystem(Game* game, const char** errmsg){
    static UserData userdata;
    const int status = SDL_Init(SDL_INIT_EVERYTHING);
    if(status){
        *errmsg = SDL_GetError();
        SDL_Quit();
        return 1;
    }
    userdata.window = SDL_CreateWindow(
        "Borderless",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SDL_WINDOW_FULLSCREEN_DESKTOP, SDL_WINDOW_FULLSCREEN_DESKTOP,
        SDL_WINDOW_RESIZABLE
    );
    if(!userdata.window){
        *errmsg = SDL_GetError();
        SDL_Quit();
        return 1;
    }
    userdata.window_surface = SDL_GetWindowSurface(userdata.window);
    if(!userdata.window_surface){
        SDL_DestroyWindow(userdata.window);
        *errmsg = SDL_GetError();
        SDL_Quit();
        return 1;
    }
    userdata.framebuffer = SDL_CreateRGBSurfaceWithFormatFrom(
        game->draw_canvas.pixels,
        game->draw_canvas.w, game->draw_canvas.h,
        8, game->draw_canvas.w * sizeof(*game->draw_canvas.pixels),
        SDL_PIXELFORMAT_RGBA32
    );
    if(!userdata.framebuffer){
        SDL_DestroyWindow(userdata.window);
        *errmsg = SDL_GetError();
        SDL_Quit();
        return 1;
    }
    game->user_data = &userdata;
    game->user_update = user_update;
    game->user_draw = user_draw;
    
    userdata.t = SDL_GetTicks64();
    return 0;
}

int quit_subsystem(Game* game, const char** errmsg){
    UserData* userdata = (UserData*) (game->user_data);
    SDL_FreeSurface(userdata->framebuffer);
    SDL_DestroyWindow(userdata->window);
    SDL_Quit();
    return 0;
}


#endif // =====================  END OF FILE BSDL_HEADER ===========================