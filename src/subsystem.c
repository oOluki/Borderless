#ifndef SUBSYSTEM_C
#define SUBSYSTEM_C

#include "subsystem.h"
#include "game.h"
#include "input.h"
#include "string.h"

static const char* const draw_mode_str[] = {
    [DRAW_MODE_NONE]    = "DRAW_MODE_NONE",
    [DRAW_MODE_GRAPHIC] = "DRAW_MODE_GRAPHIC",
    [DRAW_MODE_CONSOLE] = "DRAW_MODE_CONSOLE"
};

static char ascii_map[] = "@%#*+=-:. ";

static int getascii_color_index(Color color){
    const uint32_t rw = 2126;
    const uint32_t gw = 7152;
    const uint32_t bw =  722;

    const uint32_t r = ((color >>  0) & 0xFF);
    const uint32_t g = ((color >>  8) & 0xFF);
    const uint32_t b = ((color >> 16) & 0xFF);
    const uint32_t a = ((color >> 24) & 0xFF);

    if(!a) return 0;

    const uint32_t brightness = (rw * r + gw * g + bw * b) / (rw + gw + bw);

    return (brightness <= 255)? (brightness * (ARLEN(ascii_map) - 1)) / 255 : (ARLEN(ascii_map) - 1);
}
#ifdef __linux__

static inline void print_mem() {
    FILE *f = fopen("/proc/self/status", "r");
    char line[256];

    for (int i = 0; fgets(line, sizeof(line), f) && i < 1000; i+=1) {
        if (strncmp(line, "VmRSS:", 6) == 0) {
            printf("%s", line);
            break;
        }
    }

    fclose(f);
}
#else

#define print_mem() printf("no memory report available\n");


#endif // END OF #ifdef __linux__


#if (defined(__linux__) || defined(__APPLE__)) && !defined(MINIMAL_SETUP)

#include <unistd.h>
#include <termios.h>

static int read_key() {
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    const int c = fgetc(stdin);

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return c;
}

#elif defined(_WIN32) && !defined(MINIMAL_SETUP)

#include <conio.h>

#define read_key() _getch()

#else

#define read_key() fgetc(stdin)

#endif // read_key definition

static inline void report(){
    
    print_mem();
    printf(
        "TILE(w, h) = (%i, %i)\n"
        "entities: %i\n"
        "camera(x, y, w, h) = (%i, %i, %i, %i)\n"
        "display mode = %s\n"
        "cursor(x, y) = (%i, %i)\n"
        "player(x, y, state) = (%i, %i, %i)\n",
        TILEW, TILEH,
        game.entity_count,
        game.camera.x, game.camera.y, game.camera.w, game.camera.h,
        (game.draw_mode < ARLEN(draw_mode_str))? draw_mode_str[game.draw_mode] : "UNKWOWN",
        game.mousex, game.mousey,
        game.player.x, game.player.y, game.player.state
    );
}

int initascii_subsystem() {

    return 0;
}

int closeascii_subsystem() {

    return 0;
}

int updateascii_subsystem(){

    if(game.draw_mode == DRAW_MODE_GRAPHIC){
        printf("\x1B[2J\x1B[H\n");
        putchar('\n');
        for(int i = 0; i < game.camera.h; i+=1){
            for(int j = 0; j < game.camera.w; j+=1){
                const char ascii_char = ascii_map[getascii_color_index(game.draw_canvas.pixels[i * game.draw_canvas.stride + j])];
                putchar(ascii_char);
            }
            putchar('\n');
        }
        putchar('\n');
        fflush(stdout);
    }

    return 0;
}

int getascii_cmd(){

    static int repeat = 0;
    static int cmd = CMD_NONE;

    if(repeat > 0){
        repeat -= 1;
        return cmd;
    }

    int c = read_key();

    if(c == ':'){
        putchar(c);
        fflush(stdout);
        c = read_key();
        putchar('\r');
        switch (c)
        {
        case 'l':
            printf("\x1B[2J\x1B[H\n\r");
            return 0;
        case 'd':
            game.draw_mode = (game.draw_mode + 1) % DRAW_MODE_COUNT;
            return CMD_DISPLAY;
        case 'm':
#ifdef SUPPORT_SDL
            game.init_subsystem   = initsdl_subsystem;
            game.close_subsystem  = closesdl_subsystem;
            game.update_subsystem = updatesdl_subsystem;
            game.get_cmd          = getsdl_cmd;
            game.draw_mode   = DRAW_MODE_GRAPHIC;
            game.init_subsystem();
            game.update(CMD_DISPLAY);
            game.update_subsystem();
#else
            REPORT(REPORT, "can't change to sdl, no sdl support\n");
#endif // END OF #ifdef SUPPORT_SDL
            return CMD_NONE;
        case 'r':
            report();
            return CMD_NONE;
        case 'h':
            printf(
                "commands are sequence of characters where each character represents one command,"
                "if a character is preffixed by a number (n) than that character's command will be repeated n times, "
                "valid characters are:\n"
            );
            for(int i = 0; i < CMD_COUNT; i+=1){
                printf("\t%c: %s\n", get_cmd_char(i), get_cmd_str(i));
            }
            printf(
                "characters preffixed with - are signals, valid signals are\n"
                "\t:l: clears the whole display\n"
                "\t:d: toggles the display mode\n"
                "\t:m: toggles the subsystem mode\n"
                "\t:r: gives a report on game\n"
                "\t:h: shows this help message\n"
            );
            return CMD_NONE;
        
        default:
            REPORT(REPORT, "no signal for %u '%c', enter :h for little help message", c, c);
            return CMD_NONE;
        }
    }
    else if(c == EOF) return CMD_QUIT;

    if(c <= '9' && c >= '1'){
        repeat = 0;
        int max_size = 0;
        for(int size = 0; ; ){
            for(; c <= '9' && c >= '0'; c = read_key()){
                if(repeat * 10 + (c - '0') < 1000)
                    repeat = repeat * 10 + (c - '0');
                printf("\r");
                for(int i = 0; i < max_size; i+=1)
                    putchar(' ');
                printf("\r%i", repeat);
                fflush(stdout);
                size += 1;
            }
            // backslash
            if(size && (c == 127 || c == '\b')){
                repeat /= 10;
                //if(repeat == 0)
                //    return CMD_NONE;
                printf("\r");
                for(int i = 0; i < max_size; i+=1)
                    putchar(' ');
                printf("\r%i", repeat);
                fflush(stdout);
                c = read_key();
            }
            else break;
            max_size = MAX(max_size, size);
        }
        repeat -= !!repeat;
    }

    cmd = get_char_cmd(c);

    if(cmd == CMD_ERROR){
        REPORT(REPORT, "no cmd for char %i '%c', enter :h for little help message\n", c, c);
        repeat = 0;
        return CMD_NONE;
    }

    return cmd;
}



#ifdef SUPPORT_SDL

#include <SDL2/SDL.h>

#define WINDOW2SCREEN_SCALE_PRECISION 1000

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

    Uint64          rmouse_buttondown_time;
    Uint64          lmouse_buttondown_time;

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
        REPORT(REPORT, "SDL failed to initialize: %s\n", SDL_GetError());
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
        REPORT(REPORT, "SDL failed to create window: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    user_data.window_surface = SDL_GetWindowSurface(user_data.window);
    if(!user_data.window_surface){
        SDL_DestroyWindow(user_data.window);
        REPORT(REPORT, "SDL failed to create window_surface: %s\n", SDL_GetError());
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
        REPORT(REPORT, "SDL failed to create framebuffer: %s\n", SDL_GetError());
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
    case SDLK_q:
        return CMD_QUIT;
    case SDLK_KP_ENTER:
        if(user_data.ctrl)
            return CMD_NONE;
    case SDLK_e:
        return CMD_ENTER;
    case SDLK_b:
    case SDLK_ESCAPE:
        return CMD_BACK;
    case SDLK_UP:
    case SDLK_w:
        user_data.movement &= ~USR_UP;
        return (!user_data.continuos)? CMD_UP   : CMD_NONE;
    case SDLK_LEFT:
    case SDLK_a:
        user_data.movement &= ~USR_LEFT;
        return (!user_data.continuos)? CMD_LEFT : CMD_NONE;
    case SDLK_DOWN:
    case SDLK_s:
        user_data.movement &= ~USR_DOWN;
        return (!user_data.continuos)? CMD_DOWN : CMD_NONE;
    case SDLK_RIGHT:
    case SDLK_d:
        user_data.movement &= ~USR_RIGHT;
        return (!user_data.continuos)? CMD_RIGHT : CMD_NONE;
    case SDLK_c:
        return (!user_data.continuos)? CMD_LCLICK : CMD_NONE;
    case SDLK_v:
        return (!user_data.continuos)? CMD_RCLICK : CMD_NONE;
    case SDLK_u:
        return CMD_UPDATE;
    case SDLK_r:
        return CMD_DISPLAY;
    case SDLK_f:
        return CMD_FINNISHED;
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
    case SDLK_UP:
    case SDLK_w:
        user_data.movement |= USR_UP;
        if(user_data.continuos && user_data.chill > 160){
            user_data.chill = 0;
            return CMD_UP;
        }
        return CMD_NONE;
    case SDLK_LEFT:
    case SDLK_a:
        user_data.movement |= USR_LEFT;
        if(user_data.continuos && user_data.chill > 160){
            user_data.chill = 0;
            return CMD_LEFT;
        }
        return CMD_NONE;
    case SDLK_DOWN:
    case SDLK_s:
        user_data.movement |= USR_DOWN;
        if(user_data.continuos && user_data.chill > 160){
            user_data.chill = 0;
            return CMD_DOWN;
        }
        return CMD_NONE;
    case SDLK_RIGHT:
    case SDLK_d:
        user_data.movement |= USR_RIGHT;
        if(user_data.continuos && user_data.chill > 160){
            user_data.chill = 0;
            return CMD_RIGHT;
        }
        return CMD_NONE;
    case SDLK_c:
        if(user_data.continuos && user_data.chill > 160){
            user_data.chill = 0;
            return CMD_LCLICK;
        }
        return CMD_NONE;
    case SDLK_v:
        if(user_data.continuos && user_data.chill > 160){
            user_data.chill = 0;
            return CMD_RCLICK;
        }
        return CMD_NONE;
    case SDLK_u:
        if(user_data.continuos && user_data.chill > 160){
            user_data.chill = 0;
            return CMD_UPDATE;
        }
        return CMD_NONE;
    case SDLK_SPACE:
        if(user_data.ctrl){
            closesdl_subsystem();
            game.init_subsystem   = initascii_subsystem;
            game.close_subsystem  = closeascii_subsystem;
            game.update_subsystem = updateascii_subsystem;
            game.get_cmd          = getascii_cmd;
            game.draw_mode        = DRAW_MODE_CONSOLE;
            game.init_subsystem();
            game.update(CMD_DISPLAY);
            game.update_subsystem();
            return CMD_NONE;
        }
        return CMD_UPDATE;
    case SDLK_KP_ENTER:
        if(user_data.ctrl){
            report();
        }
        return CMD_NONE;
    case SDLK_e:
        if(user_data.continuos && user_data.chill > 160){
            user_data.chill = 0;
            return CMD_ENTER;
        }
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

int getsdl_cmd(){

    SDL_Event* const event = &user_data.event;

    while(SDL_PollEvent(event)) switch (event->type)
    {
    case SDL_QUIT:
        return CMD_QUIT;
    case SDL_KEYUP:
        return handle_keyup(event->key.keysym.sym);
    case SDL_KEYDOWN:
        return handle_keydown(event->key.keysym.sym);
    
    /*
    case SDL_MOUSEMOTION:{
        const int xscale = (WINDOW2SCREEN_SCALE_PRECISION * user_data.windoww) / game.camera.w;
        const int yscale = (WINDOW2SCREEN_SCALE_PRECISION * user_data.windowh) / game.camera.h;
        const int scale  = MIN(xscale, yscale);
        const int destw  = (scale * game.camera.w) / WINDOW2SCREEN_SCALE_PRECISION;
        const int desth  = (scale * game.camera.h) / WINDOW2SCREEN_SCALE_PRECISION;
        const int screenx = (user_data.windoww - destw) / 2;
        const int screeny = (user_data.windowh - desth) / 2;
        game.mousex = ((user_data.event.motion.x - screenx) * game.camera.w) / destw;
        game.mousey = ((user_data.event.motion.y - screeny) * game.camera.h) / desth;
    }*/
        return CMD_NONE;
    case SDL_MOUSEBUTTONDOWN:
        if(SDL_BUTTON(event->button.button) == SDL_BUTTON_LEFT)
            user_data.lmouse_buttondown_time = SDL_GetTicks64();
        else if(SDL_BUTTON(event->button.button) == SDL_BUTTON_RIGHT)
            user_data.rmouse_buttondown_time = SDL_GetTicks64();
        return CMD_NONE;
    case SDL_MOUSEBUTTONUP:
        if(SDL_BUTTON(event->button.button) == SDL_BUTTON_LEFT){
            if(SDL_GetTicks64() - user_data.lmouse_buttondown_time < 250){
                user_data.lmouse_buttondown_time = 0;
                return CMD_LCLICK;
            }
        }
        else if(SDL_BUTTON(event->button.button) == SDL_BUTTON_RIGHT){
            if(SDL_GetTicks64() - user_data.rmouse_buttondown_time < 250){
                user_data.rmouse_buttondown_time = 0;
                return CMD_RCLICK;
            }
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

    if(user_data.continuos && user_data.chill > 320){
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


#endif // END OF #ifdef SUPPORT_SDL


#endif // =====================  END OF FILE SUBSYSTEM_C ===========================