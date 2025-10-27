#ifndef ASCII_C
#define ASCII_C

#include "game.h"

static char ascii_map[] = "@%#*+=-:. ";

int getascii_color_index(Color color){
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
    }

    printf(">>> ");

    return 0;
}

int getascii_cmd(){

    int c = fgetc(stdin);

    if(c == '-'){
        c = fgetc(stdin);
        switch (c)
        {
        case 'l':
            printf("\x1B[2J\x1B[H\n");
            printf(">>> ");
            return 0;
        case 'd':
            game.draw_mode = (game.draw_mode + 1) % DRAW_MODE_COUNT;
            return CMD_NONE;
        case 'm':
#ifdef SUPPORT_SDL
            init_subsystem   = initsdl_subsystem;
            close_subsystem  = closesdl_subsystem;
            update_subsystem = updatesdl_subsystem;
            get_cmd          = getsdl_cmd;
            game.draw_mode   = DRAW_MODE_GRAPHIC;
            init_subsystem();
            game.update(CMD_DISPLAY);
            update_subsystem();
#else
            ERROR("can't change to sdl, no sdl support\n");
#endif // END OF #ifdef SUPPORT_SDL
            return CMD_NONE;
        
        default:
            VERROR("no signal for '%c'", c);
            return CMD_NONE;
        }
    }
    else if(c == EOF) return CMD_QUIT;

    const int cmd = get_char_cmd(c);

    if(cmd == CMD_ERROR){
        VERROR("no cmd for char '%c'\n", c);
        return CMD_NONE;
    }

    return cmd;
}

#endif // =====================  END OF FILE ASCII_C ===========================