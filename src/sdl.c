#ifndef SUBSYSTEM_C
#define SUBSYSTEM_C

#include <SDL2/SDL.h>
#include "game.h"


static struct UserData{

    SDL_Window*     window;
    SDL_Surface*    window_surface;
    SDL_Surface*    framebuffer;
    SDL_Event       event;
    Uint64          dt;
    Uint64          realdt;
    Uint64          t;

    Uint64          mouse_buttondown_time;

    int             continuos;
    int             ctrl;
    Uint64          chill;

} user_data;

static int handle_keyup(SDL_Keycode key){
    switch (key)
    {
    case SDLK_ESCAPE:
        return CMD_QUIT;
    case SDLK_F5:
        return CMD_RESTART;
    case SDLK_w:
        return (!user_data.continuos)? CMD_UP   : CMD_NONE;
    case SDLK_a:
        return (!user_data.continuos)? CMD_LEFT : CMD_NONE;
    case SDLK_s:
        return (!user_data.continuos)? CMD_DOWN : CMD_NONE;
    case SDLK_d:
        return (!user_data.continuos)? CMD_RIGHT : CMD_NONE;
    case SDLK_LSHIFT:
    case SDLK_RSHIFT:
        user_data.continuos = 0;
        return CMD_NONE;
    case SDLK_RCTRL:
    case SDLK_LCTRL:
        user_data.ctrl = 0;
        return CMD_DEBUG;
    default:
        return CMD_NONE;
    }
}

static int handle_keydown(SDL_Keycode key){

    switch (key)
    {
    case SDLK_w:
        if(user_data.continuos && user_data.chill > 500){
            user_data.chill = (user_data.chill > FDT)? user_data.chill - FDT : 0;
            return CMD_UP;
        }
        return CMD_NONE;
    case SDLK_a:
        if(user_data.continuos && user_data.chill > 500){
            user_data.chill = (user_data.chill > FDT)? user_data.chill - FDT : 0;
            return CMD_LEFT;
        }
        return CMD_NONE;
    case SDLK_s:
        if(user_data.continuos && user_data.chill > 500){
            user_data.chill = (user_data.chill > FDT)? user_data.chill - FDT : 0;
            return CMD_DOWN;
        }
        return CMD_NONE;
    case SDLK_d:
        if(user_data.continuos && user_data.chill > 500){
            user_data.chill = (user_data.chill > FDT)? user_data.chill - FDT : 0;
            return CMD_RIGHT;
        }
        return CMD_NONE;
    case SDLK_SPACE:
        return CMD_UPDATE;
    case SDLK_KP_ENTER:
        printf("%"PRIu64" ms; %.2f fps\n", user_data.dt, 1000.0f / (0.0f + user_data.dt));
        return CMD_NONE;
    case SDLK_RCTRL:
    case SDLK_LCTRL:
        user_data.ctrl = 1;
        return CMD_NONE;
    case SDLK_RSHIFT:
    case SDLK_LSHIFT:
        user_data.continuos = 1;
    default:
        return CMD_NONE;
    }
}

int get_cmd(){

    SDL_Event* const event = &user_data.event;

    if(SDL_PollEvent(event)) switch (event->type)
    {
    case SDL_QUIT:
        return CMD_QUIT;
    case SDL_KEYUP:
        return handle_keyup(event->key.keysym.sym);
    case SDL_KEYDOWN:
        return handle_keydown(event->key.keysym.sym);
    case SDL_MOUSEMOTION:
        game.mouse.x = (user_data.event.motion.x * game.camera.w) / user_data.window_surface->w;
        game.mouse.y = (user_data.event.motion.y * game.camera.h) / user_data.window_surface->h;
        return CMD_NONE;
    case SDL_MOUSEBUTTONDOWN:
        user_data.mouse_buttondown_time = SDL_GetTicks64();
        return CMD_NONE;
    case SDL_MOUSEBUTTONUP:
        return (SDL_GetTicks64() - user_data.mouse_buttondown_time < 30 * 16)? CMD_MOUSECLICK : CMD_NONE;
    case SDL_WINDOWEVENT:
        user_data.window_surface = SDL_GetWindowSurface(user_data.window);
        return CMD_NONE;
    
    default:
        return CMD_NONE;
    }

    return CMD_FINNISHED;
}

int update_subsystem(){

    const SDL_Rect src = (SDL_Rect){
        .x = 0,
        .y = 0,
        .w = game.camera.w,
        .h = game.camera.h
    };

    SDL_Rect dest = (SDL_Rect){
        .x = 0, .y = 0,
        .w = user_data.window_surface->w,
        .h = user_data.window_surface->h
    };

    SDL_BlitScaled(user_data.framebuffer, &src, user_data.window_surface, &dest);

    SDL_UpdateWindowSurface(user_data.window);

    { // clock
        user_data.dt = SDL_GetTicks64() - user_data.t;
        user_data.realdt = user_data.dt;
        if(user_data.dt < (1000 / FPS)){
            SDL_Delay((Uint32)((1000 / FPS) - user_data.dt));
            user_data.chill += (1000 / FPS);
            user_data.dt = 1000 / FPS;
        } else{
            user_data.chill += user_data.dt;
        }
        user_data.t = SDL_GetTicks64();
    }

    return 0;
}

int init_subsystem(){
    const int status = SDL_Init(SDL_INIT_EVERYTHING);
    if(status){
        VERROR("SDL failed to initialize: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    user_data.window = SDL_CreateWindow(
        "Borderless",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SDL_WINDOW_FULLSCREEN_DESKTOP, SDL_WINDOW_FULLSCREEN_DESKTOP,
        SDL_WINDOW_RESIZABLE
    );
    if(!user_data.window){
        VERROR("SDL failed to create window: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    user_data.window_surface = SDL_GetWindowSurface(user_data.window);
    if(!user_data.window_surface){
        SDL_DestroyWindow(user_data.window);
        VERROR("SDL failed to create window_surface: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    user_data.framebuffer = SDL_CreateRGBSurfaceWithFormatFrom(
        game.draw_canvas.pixels,
        game.draw_canvas.w, game.draw_canvas.h,
        8, game.draw_canvas.w * sizeof(*game.draw_canvas.pixels),
        SDL_PIXELFORMAT_RGBA32
    );
    if(!user_data.framebuffer){
        SDL_DestroyWindow(user_data.window);
        VERROR("SDL failed to create framebuffer: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_ShowCursor(SDL_ENABLE);
    
    user_data.t = SDL_GetTicks64();
    return 0;
}

int close_subsystem(){
    SDL_FreeSurface(user_data.framebuffer);
    SDL_DestroyWindow(user_data.window);
    SDL_Quit();
    return 0;
}

#endif // END OF FILE SUBSYSTEM_C ======================================================================