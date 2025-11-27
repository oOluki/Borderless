#ifndef SUBSYSTEM_C
#define SUBSYSTEM_C

#include <SDL2/SDL.h>
#include "game.h"

#define WINDOW2SCREEN_SCALE_PRECISION 1000

extern int initascii_subsystem();
extern int closeascii_subsystem();
extern int updateascii_subsystem();
extern int getascii_cmd();

static struct UserData{

    SDL_Window*     window;
    SDL_Surface*    window_surface;
    SDL_Surface*    framebuffer;
    int             windoww;
    int             windowh;
    SDL_Event       event;
    Uint64          dt;
    Uint64          realdt;
    Uint64          t;

    Uint64          mouse_buttondown_time;

    int             continuos;
    int             ctrl;
    Uint64          chill;

    int             movement;

} user_data;

enum UserMovement{
    USR_UP    = 1 << 0,
    USR_LEFT  = 1 << 1,
    USR_DOWN  = 1 << 2,
    USR_RIGHT = 1 << 3
};


int initsdl_subsystem(){
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

int closesdl_subsystem(){
    SDL_FreeSurface(user_data.framebuffer);
    SDL_DestroyWindow(user_data.window);
    SDL_Quit();
    return 0;
}

static inline int get_cmd_from_movement(int movement){
    if((movement & USR_LEFT)  && !(movement & USR_RIGHT)) return CMD_LEFT;
    if((movement & USR_RIGHT) && !(movement & USR_LEFT) ) return CMD_RIGHT;
    if((movement & USR_UP)    && !(movement & USR_DOWN) ) return CMD_UP;
    if((movement & USR_DOWN)  && !(movement & USR_UP)   ) return CMD_DOWN;
    
    return CMD_NONE;
}

static int handle_keyup(SDL_Keycode key){
    switch (key)
    {
    case SDLK_ESCAPE:
        return CMD_BACK;
    case SDLK_F5:
        return CMD_CHEAT_RESTART;
    case SDLK_w:
        user_data.movement &= ~USR_UP;
        return (!user_data.continuos)? CMD_UP   : CMD_NONE;
    case SDLK_a:
        user_data.movement &= ~USR_LEFT;
        return (!user_data.continuos)? CMD_LEFT : CMD_NONE;
    case SDLK_s:
        user_data.movement &= ~USR_DOWN;
        return (!user_data.continuos)? CMD_DOWN : CMD_NONE;
    case SDLK_d:
        user_data.movement &= ~USR_RIGHT;
        return (!user_data.continuos)? CMD_RIGHT : CMD_NONE;
    case SDLK_l:
        if(user_data.ctrl){
            printf("\x1B[2J\x1B[H\n");
        }
        return CMD_NONE;
    case SDLK_LSHIFT:
    case SDLK_RSHIFT:
        user_data.continuos = 0;
        return CMD_NONE;
    case SDLK_RCTRL:
    case SDLK_LCTRL:
        user_data.ctrl = 0;
        return CMD_NONE;
    default:
        return CMD_NONE;
    }
}

static int handle_keydown(SDL_Keycode key){

    switch (key)
    {
    case SDLK_w:
        user_data.movement |= USR_UP;
        if(user_data.continuos && user_data.chill > 160){
            user_data.chill = 0;
            return CMD_UP;
        }
        return CMD_NONE;
    case SDLK_a:
        user_data.movement |= USR_LEFT;
        if(user_data.continuos && user_data.chill > 160){
            user_data.chill = 0;
            return CMD_LEFT;
        }
        return CMD_NONE;
    case SDLK_s:
        user_data.movement |= USR_DOWN;
        if(user_data.continuos && user_data.chill > 160){
            user_data.chill = 0;
            return CMD_DOWN;
        }
        return CMD_NONE;
    case SDLK_d:
        user_data.movement |= USR_RIGHT;
        if(user_data.continuos && user_data.chill > 160){
            user_data.chill = 0;
            return CMD_RIGHT;
        }
        return CMD_NONE;
    case SDLK_SPACE:
        if(user_data.ctrl){
            closesdl_subsystem();
            init_subsystem   = initascii_subsystem;
            close_subsystem  = closeascii_subsystem;
            update_subsystem = updateascii_subsystem;
            get_cmd          = getascii_cmd;
            init_subsystem();
            game.update(CMD_DISPLAY);
            update_subsystem();
            return CMD_NONE;
        }
        return CMD_UPDATE;
    case SDLK_KP_ENTER:
        if(user_data.ctrl){
            printf("%"PRIu64" ms; %.2f fps\nctrl: %i\nentities: %i\n",
                user_data.dt, 1000.0f / (0.0f + user_data.dt),
                user_data.ctrl,
                game.entity_count
            );
            return CMD_NONE;
        }
        return CMD_ENTER;
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

int getsdl_cmd(){

    SDL_Event* const event = &user_data.event;

    if(SDL_PollEvent(event)) switch (event->type)
    {
    case SDL_QUIT:
        return CMD_QUIT;
    case SDL_KEYUP:
        return handle_keyup(event->key.keysym.sym);
    case SDL_KEYDOWN:
        return handle_keydown(event->key.keysym.sym);
    case SDL_MOUSEMOTION:{
        const int xscale = (WINDOW2SCREEN_SCALE_PRECISION * user_data.windoww) / game.camera.w;
        const int yscale = (WINDOW2SCREEN_SCALE_PRECISION * user_data.windowh) / game.camera.h;
        const int scale  = MIN(xscale, yscale);
        const int destw  = (scale * game.camera.w) / WINDOW2SCREEN_SCALE_PRECISION;
        const int desth  = (scale * game.camera.h) / WINDOW2SCREEN_SCALE_PRECISION;
        const int screenx = (user_data.windoww - destw) / 2;
        const int screeny = (user_data.windowh - desth) / 2;
        game.mouse.x = ((user_data.event.motion.x - screenx) * game.camera.w) / destw;
        game.mouse.y = ((user_data.event.motion.y - screeny) * game.camera.h) / desth;
        for(int i = 0; i < game.button_count; i+=1){
            const int y0 = ((i + 0) * game.camera.h) / game.button_count;
            const int y1 = ((i + 1) * game.camera.h) / game.button_count;
            if(game.mouse.y >= y0 && game.mouse.y <= y1){
                game.selected_button = i;
            }
        }
    }
        return CMD_NONE;
    case SDL_MOUSEBUTTONDOWN:
        user_data.mouse_buttondown_time = SDL_GetTicks64();
        return CMD_NONE;
    case SDL_MOUSEBUTTONUP:
        if(SDL_GetTicks64() - user_data.mouse_buttondown_time < 250){
            user_data.mouse_buttondown_time = 0;
            return CMD_MOUSECLICK;
        }
        return CMD_NONE;
    case SDL_WINDOWEVENT:
        if(event->window.event == SDL_WINDOWEVENT_RESIZED){
            user_data.windoww = (int) event->window.data1;
            user_data.windowh = (int) event->window.data2;
        }
        user_data.window_surface = SDL_GetWindowSurface(user_data.window);
        return CMD_NONE;
    
    default:
        break;;
    }

    if(user_data.continuos && user_data.chill > 160){
        user_data.chill = 0;
        return get_cmd_from_movement(user_data.movement);
    }

    return CMD_FINNISHED;
}

int updatesdl_subsystem(){

    const int xscale = (WINDOW2SCREEN_SCALE_PRECISION * user_data.windoww) / game.camera.w;
    const int yscale = (WINDOW2SCREEN_SCALE_PRECISION * user_data.windowh) / game.camera.h;
    const int  scale = MIN(xscale, yscale);

    const int destw = (scale * game.camera.w) / WINDOW2SCREEN_SCALE_PRECISION;
    const int desth = (scale * game.camera.h) / WINDOW2SCREEN_SCALE_PRECISION;

    const SDL_Rect src = (SDL_Rect){
        .x = 0,
        .y = 0,
        .w = game.camera.w,
        .h = game.camera.h
    };

    SDL_Rect dest = (SDL_Rect){
        .x = (user_data.windoww - destw) / 2,
        .y = (user_data.windowh - desth) / 2,
        .w = destw,
        .h = desth
    };

    const SDL_Rect background0 = (SDL_Rect){
        .x = 0, .y = 0,
        .w = dest.x, .h = user_data.windowh
    };
    const SDL_Rect background1 = (SDL_Rect){
        .x = dest.x + dest.w, .y = 0,
        .w = dest.x, .h = user_data.windowh
    };
    const SDL_Rect background2 = (SDL_Rect){
        .x = dest.x, .y = 0,
        .w = user_data.windoww - 2 * dest.x, .h = dest.y
    };
    const SDL_Rect background3 = (SDL_Rect){
        .x = dest.x, .y = dest.y + dest.h,
        .w = user_data.windoww - 2 * dest.x, .h = dest.y
    };

    const SDL_Rect backgrounds[] = {
        background0, background1, background2, background3
    };

    SDL_FillRects(user_data.window_surface, backgrounds, ARLEN(backgrounds), 0xFF000000);

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

#endif // END OF FILE SUBSYSTEM_C ======================================================================