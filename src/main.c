#include "game.c"


#ifdef SUPPORT_SDL

    #include "sdl.c"

#endif


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

    #define MAIN_RETURN_STATUS(STATUS) do { status = STATUS; goto defer; } while(0)

    static Pixel draw_canvas_pixels[800 * 640];

    FILE* input  = NULL;
    FILE* output = NULL;

    int draw_mode_changed = 0;

    int display_final_state = 0;

    int status = 0;

// ==============================[parsing command line arguments]==========================================

    for(int i = 1; i < argc; i+=1){
        if(cmp_str(argv[i], "--help", 0)){
            printf(
                "simple game\n"
                "usage: %s -<optional: flags>... --<optional: kwargs>...\n"
                "flags can be:\n"
                "\tc: draw in console simple mode\n"
                "\tg: draw in graphical mode\n"
                "\tn: don't draw at all\n"
                "\tf: draw final state (even if the n flag is active)\n"
                "\ti: take input from stdin\n"
                "\to: print commands to stdout\n"
                "kwargs can be:\n"
                "\tinput <file>: take input from file\n"
                "\toutput <file>: record commands to file, 'record' is equivalent to 'output'\n"
                "\twidth=<width>: sets the camera width\n"
                "\theight=<heigth>: sets the camera height\n"
#ifdef SUPPORT_SDL
                "\tuse_sdl: use sdl to render graphics\n"
#endif
                "\tuse_ascii: use ascii to render graphics\n",
                argv[0]
            );
            MAIN_RETURN_STATUS(0);
        }
        else if(cmp_str(argv[i], "--input", 0)){
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
                    init_subsystem   = initascii_subsystem;
                    close_subsystem  = closeascii_subsystem;
                    update_subsystem = updateascii_subsystem;
                    get_cmd          = getascii_cmd;
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
                case 'f':
                    display_final_state = 1;
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
        get_cmd = getascii_cmd;
#endif
    }

    if(draw_mode_changed == 0){
        game.draw_mode = DRAW_MODE_GRAPHIC;
    }

    if(init_subsystem()){
        MAIN_RETURN_STATUS(1);
    }

// ==============================[end of parsing command line arguments]==========================================

    game.update(CMD_DISPLAY);

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
                fprintf(stderr, "[ERROR] invalid command char %u '%c'\n", c, c);
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

    if(display_final_state) {
	    game.draw_mode = DRAW_MODE_CONSOLE;
	    draw();
        update_subsystem();
    }

    defer:
    if(input)  fclose(input);
    if(output && output != stdout) fclose(output);

    if(close_subsystem) if(close_subsystem()){
        return 1;
    }

    return status;
}
