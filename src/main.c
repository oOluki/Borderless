#include "game.c"
#include "sdl.c"

static char get_cmd_char(int cmd){
    switch (cmd)
    {
    case CMD_NONE:              return ' ';
    case CMD_QUIT:              return 'q';
    case CMD_UPDATE:            return 'u';
    case CMD_DEBUG:             return 'D';
    case CMD_RESTART:           return 'r';
    case CMD_ENTER:             return 'e';
    case CMD_PAUSE:             return 'p';
    case CMD_MOUSECLICK:        return 'c';
    case CMD_UP:                return 'w';
    case CMD_RIGHT:             return 'd';
    case CMD_LEFT:              return 'a';
    case CMD_DOWN:              return 's';
    case CMD_CHEAT_REVIVE:      return '@';
    case CMD_SPECIAL_SIGNAL:    return '^';
    case CMD_FINNISHED:         return '\n';
    default:                    return '?';
    }
}

static int get_char_cmd(int _char){
    switch (_char)
    {
    case '\t':
    case ' ':   return CMD_NONE;
    case EOF:
    case 'q':   return CMD_QUIT;
    case 'u':   return CMD_UPDATE;
    case 'D':   return CMD_DEBUG;
    case 'r':   return CMD_RESTART;
    case 'e':   return CMD_ENTER;
    case 'p':   return CMD_PAUSE;
    case 'c':   return CMD_MOUSECLICK;
    case 'w':   return CMD_UP;
    case 'd':   return CMD_RIGHT;
    case 'a':   return CMD_LEFT;
    case 's':   return CMD_DOWN;
    case '@':   return CMD_CHEAT_REVIVE;
    case '^':   return CMD_SPECIAL_SIGNAL;
    case '\0':
    case '\n':  return CMD_FINNISHED;
    default:    return CMD_ERROR;
    }
}

static int cmp_str(const char* str1, const char* str2, int only_compare_untill_first_null){
    if(!str1 || !str2) return 0;
    while(*str1 && *str1 == *str2){
        str1 += 1;
        str2 += 1;
    }
    return (*str1 == *str2) || (only_compare_untill_first_null && (!*str1 || !*str2));
}


int main(int argc, char** argv){

    static Pixel draw_canvas_pixels[800 * 600];

    FILE* input  = NULL;
    FILE* output = NULL;

    int subsystem_initiated = 0;

    int status = 0;

    for(int i = 1; i < argc; i+=1){
        if(cmp_str(argv[i], "--input", 0)){
            if(i + 1 >= argc){
                fprintf(stderr, "[ERROR] missing input file after \'--input\'\n");
                status = 1;
                goto defer;
            }
            if(input){
                fprintf(stderr, "[ERROR] multiple input files given, NOTE that you can not take input from both stdin (using the i flag) and another file\n");
                status = 1;
                goto defer;
            }
            input = fopen(argv[++i], "r");
            if(!input){
                fprintf(stderr, "[ERROR] could not open input file '%s'\n", argv[i]);
                status = 1;
                goto defer;
            }
        }
        else if(cmp_str(argv[i], "--record", 0) || cmp_str(argv[i], "--output", 0)){
            if(i + 1 >= argc){
                fprintf(stderr, "[ERROR] missing output file after '%s'\n", argv[i]);
                status = 1;
                goto defer;
            }
            if(output){
                fprintf(stderr, "[ERROR] multiple output files given, NOTE that you cannot use both stdout (using the o flag) and another file as output\n");
                status = 1;
                goto defer;
            }
            output = fopen(argv[++i], "w");
            if(!output){
                fprintf(stderr, "[ERROR] could not open output file '%s'\n", argv[i]);
                status = 1;
                goto defer;
            }
        }
        else if(cmp_str(argv[i], "--palette", 0)){
            if(i + 1 >= argc){
                fprintf(stderr, "[ERROR] missing palette after --palette\n");
                status = 1;
                goto defer;
            }
            int palette_len = 0;
            for(i+=1; argv[i][palette_len]; palette_len+=1);
            if(palette_len != ARLEN(console_palette)){
                fprintf(stderr, (palette_len < ARLEN(console_palette))? "[ERROR] palette missing cases\n" : "[ERROR] palette has too many terms\n");
                status = 1;
                goto defer;
            }
            char* opalette = (char*) console_palette;
            palette_len = 0;
            for(int j = 0; j < ARLEN(console_palette); j+=1){
                for(int k = 0; k < palette_len; k+=1){
                    if(console_palette[k] == argv[i][j]){
                        fprintf(stderr, "[ERROR] palette has multiple '%c' term\n", argv[i][j]);
                        status = 1;
                        goto defer;
                    }
                }
                opalette[j] = argv[i][j];
            }
        }
        else if(cmp_str(argv[i], "-s", 1)){
            int len = 0;
            for(; argv[i][len] && len < 5; len+=1);
            if(len > 4){
                fprintf(stderr, "[ERROR] invalid use of -s<old><new>: '%s'\n", argv[i]);
                status = 1;
                goto defer;
            }
            int found = 0;
            for(int j = 0; j < ARLEN(console_palette); j+=1){
                if(console_palette[j] == argv[i][2]){
                    found = 1;
                    ((char*) (console_palette))[j] = argv[i][3];
                    break;
                }
            }
            if(!found){
                fprintf(stderr, "[ERROR] '%c' symbol not found in palette\n", argv[i][2]);
                status = 1;
                goto defer;
            }
        }
        else if(argv[i][0] == '-'){
            for(int j = 1; argv[i][j]; j+=1){
                switch (argv[i][j])
                {
                case 'i':
                    if(input && input != stdin){
                        fprintf(stderr, "[ERROR] can't take input from both stdin and another file\n");
                        status = 1;
                        goto defer;
                    }
                    input = stdin;
                    break;
                case 'o':
                    if(output && output != stdout){
                        fprintf(stderr, "[ERROR] use both stdout and another file as output\n");
                        status = 1;
                        goto defer;
                    }
                    output = stdout;
                    break;
                case 'c':
                    game.console_mode = 1;
                    break;
                
                default:
                    fprintf(stderr, "[ERROR] invvalid flag '%c' in '%s'\n", argv[i][j], argv[i]);
                    status = 1;
                    goto defer;
                }
            }
        }
        else{
            if(output){
                fprintf(stderr, "[ERROR] multiple output files given, NOTE that you cannot use both stdout (using the o flag) and another file as output\n");
                status = 1;
                goto defer;
            }
            output = fopen(argv[i], "w");
            if(!output){
                fprintf(stderr, "[ERROR] could not open output file '%s'\n", argv[i]);
                status = 1;
                goto defer;
            }
        }
    }

    if(game_init(draw_canvas_pixels, 800, 600, 800)){
        status = 1;
        goto defer;
    }

    if(!game.console_mode){
        if(init_subsystem()){
            status = 1;
            goto defer;
        }
        subsystem_initiated = 1;
    }

    game.update(CMD_UPDATE);

    if(input){ // take input from file

        while(game.active){

            int c = fgetc(input);
            int cmd = get_char_cmd(c);

            while(cmd != CMD_FINNISHED && cmd != CMD_ERROR){
                printf("%c\n", c);
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
    if(game.console_mode) while(game.active){

            printf(">>> ");

            int c = fgetc(stdin);
            int cmd = get_char_cmd(c);

            while(cmd != CMD_FINNISHED && cmd != CMD_ERROR){

                if(output && cmd != CMD_NONE) fputc((int) get_cmd_char(cmd), output);

                if(game.update(cmd)) break;

                c = fgetc(stdin);
                cmd = get_char_cmd(c);
            }
            if(cmd == CMD_ERROR){
                fprintf(stderr, "[ERROR] invalid command char '%c'\n", c);
                continue;
            }
            else if(output && cmd != CMD_NONE) fputc((int) get_cmd_char(cmd), output);
    }
    else while(game.active){

        int cmd = get_cmd();

        for(; cmd != CMD_FINNISHED; cmd = get_cmd()){

            if(output && cmd != CMD_NONE) fputc((int) get_cmd_char(cmd), output);

            if(game.update(cmd)) break;
        }
        if(output && cmd != CMD_NONE) fputc((int) get_cmd_char(cmd), output);
        update_subsystem();
    }

    defer:

    if(input)  fclose(input);
    if(output) fclose(output);

    if(subsystem_initiated) if(close_subsystem()){
        return 1;
    }

    return status;
}
