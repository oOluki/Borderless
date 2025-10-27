#include "game.c"


static int (*init_subsystem)();
static int (*close_subsystem)();
static int (*update_subsystem)();
static int (*get_cmd)();


int initascii_subsystem();
int closeascii_subsystem();
int updateascii_subsystem();
int getascii_cmd();


#ifdef SUPPORT_SDL

    int initsdl_subsystem();
    int closesdl_subsystem();
    int updatesdl_subsystem();
    int getsdl_cmd();

    #include "sdl.c"

#endif

static inline int get_hex_digit(char c){
    switch (c)
    {
    case '0': return 0;
    case '1': return 1;
    case '2': return 2;
    case '3': return 3;
    case '4': return 4;
    case '5': return 5;
    case '6': return 6;
    case '7': return 7;
    case '8': return 8;
    case '9': return 9;
    case 'a':
    case 'A': return 10;
    case 'b':
    case 'B': return 11;
    case 'c':
    case 'C': return 12;
    case 'd':
    case 'D': return 13;
    case 'e':
    case 'E': return 14;
    case 'f':
    case 'F': return 15;
    
    default: return -1;
    }
}

static int64_t parse_hex32(const char* str){
    int64_t output = 0;
    int i = 0;
    int d = 0;
    for(int i = 0; i < 8 && d >= 0; i+=1){
        d = get_hex_digit(str[i]);
        output = (output * 16) + d;
    }
    return (d < 0)? -1 : output;
}

#include "ascii.c"



static int cmp_str(const char* str1, const char* str2, int only_compare_untill_first_null){
    if(!str1 || !str2) return 0;
    while(*str1 && *str1 == *str2){
        str1 += 1;
        str2 += 1;
    }
    return (*str1 == *str2) || (only_compare_untill_first_null && (!*str1 || !*str2));
}


int main(int argc, char** argv){

    #define MAIN_RETURN_STATUS(STATUS) do { status = 1; goto defer; } while(0)

    static Pixel draw_canvas_pixels[800 * 600];

    FILE* input  = NULL;
    FILE* output = NULL;

    int draw_mode_changed = 0;

    int status = 0;

// ==============================[parsing command line arguments]==========================================

    for(int i = 1; i < argc; i+=1){
        if(cmp_str(argv[i], "--input", 0)){
            if(i + 1 >= argc){
                fprintf(stderr, "[ERROR] missing input file after \'--input\'\n");
                MAIN_RETURN_STATUS(1);
            }
            if(input){
                fprintf(stderr, "[ERROR] multiple input files given, NOTE that you can not take input from both stdin (using the i flag) and another file\n");
                MAIN_RETURN_STATUS(1);
            }
            input = fopen(argv[++i], "r");
            if(!input){
                fprintf(stderr, "[ERROR] could not open input file '%s'\n", argv[i]);
                MAIN_RETURN_STATUS(1);
            }
        }
        else if(cmp_str(argv[i], "--record", 0) || cmp_str(argv[i], "--output", 0)){
            if(i + 1 >= argc){
                fprintf(stderr, "[ERROR] missing output file after '%s'\n", argv[i]);
                MAIN_RETURN_STATUS(1);
            }
            if(output){
                fprintf(stderr, "[ERROR] multiple output files given, NOTE that you cannot use both stdout (using the o flag) and another file as output\n");
                MAIN_RETURN_STATUS(1);
            }
            output = fopen(argv[++i], "w");
            if(!output){
                fprintf(stderr, "[ERROR] could not open output file '%s'\n", argv[i]);
                MAIN_RETURN_STATUS(1);
            }
        }
        else if(cmp_str(argv[i], "--width=", 1)){
            int width = 0;
            int j = ARLEN("--width=") - 1;
            for(; argv[i][j] >= '0' && argv[i][j] <= '9'; j+=1){
                width = (width * 10) + (argv[i][j] - '0');
            }
            if(argv[i][j]){
                fprintf(stderr, "[ERROR] invalid width in %s\n", argv[i]);
                MAIN_RETURN_STATUS(1);
            }
            game.camera.w = width;
        }
        else if(cmp_str(argv[i], "--height=", 1)){
            int height = 0;
            int j = ARLEN("--width=") - 1;
            for(; argv[i][j] >= '0' && argv[i][j] <= '9'; j+=1){
                height = (height * 10) + (argv[i][j] - '0');
            }
            if(argv[i][j]){
                fprintf(stderr, "[ERROR] invalid height in %s\n", argv[i]);
                MAIN_RETURN_STATUS(1);
            }
            game.camera.h = height;
        }
        else if(cmp_str(argv[i], "--palette", 0)){
            if(i + 1 >= argc){
                fprintf(stderr, "[ERROR] missing palette after --palette\n");
                MAIN_RETURN_STATUS(1);
            }
            int palette_len = 0;
            for(i+=1; argv[i][palette_len]; palette_len+=1);
            if(palette_len != ARLEN(console_palette)){
                fprintf(stderr, (palette_len < ARLEN(console_palette))? "[ERROR] palette missing cases\n" : "[ERROR] palette has too many terms\n");
                MAIN_RETURN_STATUS(1);
            }
            char* opalette = (char*) console_palette;
            palette_len = 0;
            for(int j = 0; j < ARLEN(console_palette); j+=1){
                for(int k = 0; k < palette_len; k+=1){
                    if(console_palette[k] == argv[i][j]){
                        fprintf(stderr, "[ERROR] palette has multiple '%c' term\n", argv[i][j]);
                        MAIN_RETURN_STATUS(1);
                    }
                }
                opalette[j] = argv[i][j];
            }
        }
        else if(cmp_str(argv[i], "--s", 1)){
            const int __s_len = sizeof("--s") / sizeof(char) - 1;
            int len = 0;
            for(; argv[i][len] && len < __s_len + 3; len+=1);
            if(len != __s_len + 2){
                fprintf(stderr, "[ERROR] invalid use of %.*s<old><new>: '%s'\n", __s_len, argv[i], argv[i]);
                MAIN_RETURN_STATUS(1);
            }
            int found = 0;
            for(int j = 0; j < ARLEN(console_palette); j+=1){
                if(console_palette[j] == argv[i][__s_len]){
                    found = 1;
                    ((char*) (console_palette))[j] = argv[i][__s_len + 1];
                    break;
                }
            }
            if(!found){
                fprintf(stderr, "[ERROR] '%c' symbol not found in palette\n", argv[i][2]);
                MAIN_RETURN_STATUS(1);
            }
        }
        else if(cmp_str(argv[i], "--use_sdl", 0)){
#ifdef SUPPORT_SDL
            init_subsystem   = initsdl_subsystem;
            close_subsystem  = closesdl_subsystem;
            update_subsystem = updatesdl_subsystem;
            get_cmd          = getsdl_cmd;
#else
            fprintf(stderr, "[ERROR] can't use '%s': no support for sdl\n", argv[i]);
            MAIN_RETURN_STATUS(1);
#endif
        }
        else if(cmp_str(argv[i], "--use_ascii", 0)){
            init_subsystem   = initascii_subsystem;
            close_subsystem  = closeascii_subsystem;
            update_subsystem = updateascii_subsystem;
            get_cmd          = getascii_cmd;
        }
        else if(cmp_str(argv[i], "--ascii_swap", 1)){
            const int ascii_swap_len = sizeof("--ascii_swap") / sizeof(char) - 1;
            int len = 0;
            for(; argv[i][len] && len < ascii_swap_len + 2; len+=1);
            if(len != ascii_swap_len + 2){
                fprintf(stderr, "[ERROR] invalid use of %.*s<old><new>: '%s'\n", ascii_swap_len, argv[i], argv[i]);
                MAIN_RETURN_STATUS(1);
            }
        }
        else if(cmp_str(argv[i], "--ascii_set", 1)){
            const int ascii_swap_len = sizeof("--ascii_set") / sizeof(char) - 1;
            int len = ascii_swap_len;
            for(; argv[i][len] && len < ascii_swap_len + 10; len+=1);
            if(len != ascii_swap_len + 10){
                fprintf(stderr,
                    "[ERROR] invalid use of %.*s<color>=<char>: '%s'\n"
                    "[NOTE] color has to be in hexadecimal representation with all 4 channels (RGBA) present\n",
                    ascii_swap_len, argv[i], argv[i]);
                MAIN_RETURN_STATUS(1);
            }
            const int64_t color = parse_hex32(&argv[i][ascii_swap_len]);
            if(color < 0){
                fprintf(stderr, "[ERROR] Invalid color '%.*s' in '%s'\n", 8, &argv[i][ascii_swap_len], argv[i]);
                MAIN_RETURN_STATUS(1);
            }
            if(argv[i][ascii_swap_len + 8] != '='){
                fprintf(stderr, "[ERROR] missing '=' in %s\n", argv[i]);
                MAIN_RETURN_STATUS(1);
            }
            const int index = getascii_color_index(color);
            ascii_map[index] = argv[i][ascii_swap_len + 9];
        }
        else if(argv[i][0] == '-'){
            for(int j = 1; argv[i][j]; j+=1){
                switch (argv[i][j])
                {
                case 'c':
                    draw_mode_changed = 1;
                    init_subsystem   = initascii_subsystem;
                    close_subsystem  = closeascii_subsystem;
                    update_subsystem = updateascii_subsystem;
                    get_cmd          = getascii_cmd;
                    game.draw_mode = DRAW_MODE_CONSOLE;
                    break;
                case 'g':
                    draw_mode_changed = 1;
#ifdef SUPPORT_SDL
                    init_subsystem   = initsdl_subsystem;
                    close_subsystem  = closesdl_subsystem;
                    update_subsystem = updatesdl_subsystem;
                    get_cmd          = getsdl_cmd;
#else
                    init_subsystem   = initascii_subsystem;
                    close_subsystem  = closeascii_subsystem;
                    update_subsystem = updateascii_subsystem;
                    get_cmd          = getascii_cmd;
#endif
                    game.draw_mode = DRAW_MODE_GRAPHIC;
                    break;
                case 'n':
                    draw_mode_changed = 1;
                    game.draw_mode = DRAW_MODE_NONE;
                    break;
                case 'i':
                    if(input && input != stdin){
                        fprintf(stderr, "[ERROR] can't take input from both stdin and another file\n");
                        MAIN_RETURN_STATUS(1);
                    }
                    input = stdin;
                    break;
                case 'o':
                    if(output && output != stdout){
                        fprintf(stderr, "[ERROR] can't use both stdout and another file as output\n");
                        MAIN_RETURN_STATUS(1);
                    }
                    output = stdout;
                    break;
                
                default:
                    fprintf(stderr, "[ERROR] invalid flag '%c' in '%s'\n", argv[i][j], argv[i]);
                    MAIN_RETURN_STATUS(1);
                }
            }
        }
        else{
            if(output){
                fprintf(stderr, "[ERROR] multiple output files given, NOTE that you cannot use both stdout (using the o flag) and another file as output\n");
                MAIN_RETURN_STATUS(1);
            }
            output = fopen(argv[i], "w");
            if(!output){
                fprintf(stderr, "[ERROR] could not open output file '%s'\n", argv[i]);
                MAIN_RETURN_STATUS(1);
            }
        }
    }

    if(game_init(draw_canvas_pixels, 800, 600)){
        MAIN_RETURN_STATUS(1);
    }

    if(!init_subsystem){
#ifdef SUPPORT_SDL
        init_subsystem = initsdl_subsystem;
#else
        init_subsystem = initascii_subsystem;
#endif
    }
    if(!close_subsystem){
#ifdef SUPPORT_SDL
        close_subsystem = closesdl_subsystem;
#else
        close_subsystem = closeascii_subsystem;
#endif
    }
    if(!update_subsystem){
#ifdef SUPPORT_SDL
        update_subsystem = updatesdl_subsystem;
#else
        update_subsystem = updateascii_subsystem;
#endif
    }
    if(!get_cmd){
#ifdef SUPPORT_SDL
        get_cmd = getsdl_cmd;
#else
        get_cmd = getsdl_cmd;
#endif
    }

    if(draw_mode_changed == 0){
        game.draw_mode = DRAW_MODE_GRAPHIC;
    }

    if(init_subsystem()){
        MAIN_RETURN_STATUS(1);
    }

// ==============================[end of parsing command line arguments]==========================================

    game.update(CMD_UPDATE);

    if(input){ // take input from file

        while(game.active && !feof(input)){

            int c = fgetc(input);
            int cmd = get_char_cmd(c);

            while(cmd != CMD_FINNISHED && cmd != CMD_ERROR){
                
                if(output && cmd != CMD_NONE) fputc((int) get_cmd_char(cmd), output);

                if(game.update(cmd)) break;

                c = fgetc(input);
                cmd = get_char_cmd(c);
            }
            if(cmd == CMD_ERROR){
                fprintf(stderr, "[ERROR] invalid command char '%c'\n", c);
                break;
            }
            else if(output && cmd != CMD_NONE) fputc((int) get_cmd_char(cmd), output);
        }
    }
    while(game.active){

        int cmd = get_cmd();

        for(; cmd != CMD_FINNISHED && cmd != CMD_ERROR; cmd = get_cmd()){

            if(output && cmd != CMD_NONE) fputc((int) get_cmd_char(cmd), output);

            if(game.update(cmd)) break;
        }
        if(output && cmd != CMD_NONE) fputc((int) get_cmd_char(cmd), output);
        update_subsystem();
    }

    defer:
    if(input)  fclose(input);
    if(output) fclose(output);

    if(close_subsystem()){
        return 1;
    }

    return status;
}
