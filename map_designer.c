#include <stdio.h>
#include <stdlib.h>

#define TILE_SIZE 3
#define MAP_MAGIC_WORD 0x6d617000

#define MAX_MAPW 1000
#define MAX_MAPH 1000
#define MAP_CAP (MAX_MAPW * MAX_MAPH)

#define MAP_PATH_CAP 1024

#define BUFF_CAP 1024

enum Tiles{
    TILE_EMPTY = 0,
    TILE_FIRST_TILE_ID = TILE_EMPTY,
    TILE_WALL,
    TILE_SWALL,

    TILE_LAST_TILE_ID = 15,

    TILE_FIRST_ENTITY_ID = 16,
    TILE_PLAYER_FACE_UP = TILE_FIRST_ENTITY_ID,
    TILE_PLAYER_FACE_RIGHT,
    TILE_PLAYER_FACE_DOWN,
    TILE_PLAYER_FACE_LEFT,

    TILE_ENEMY1_FACE_UP,
    TILE_ENEMY1_FACE_RIGHT,
    TILE_ENEMY1_FACE_DOWN,
    TILE_ENEMY1_FACE_LEFT,

    TILE_LAST_ENTITY_ID = 31,

    // for counting purposes
    TILE_COUNT = 32
};

static char map[MAP_CAP];
static int  mapw = 0;
static int  maph = 0;

static char map2[MAP_CAP];

static char  map_path_buff[MAP_PATH_CAP];
static char* map_path = NULL;
static int   map_path_size = 0;

static int camerax = 0;
static int cameray = 0;
static int cameraw = 32;
static int camerah = 24;

static char framebuff[100 * 100];
static int  framebuffw = 100;
static int  framebuffh = 100;

static int pencilw = 1;
static int pencilh = 1;

static int copyx = -1;
static int copyy = -1;

static char buff[BUFF_CAP];
static int  buffsize = 0;

static char palette[] = {
    [TILE_EMPTY]                =   ' ',
    [TILE_WALL]                 =   'X',
    [TILE_SWALL]                =   '#',
    [TILE_PLAYER_FACE_UP]       =   'p',
    [TILE_PLAYER_FACE_RIGHT]    =   'p',
    [TILE_PLAYER_FACE_DOWN]     =   'p',
    [TILE_PLAYER_FACE_LEFT]     =   'p',
    [TILE_ENEMY1_FACE_UP]       =   'e',
    [TILE_ENEMY1_FACE_RIGHT]    =   'e',
    [TILE_ENEMY1_FACE_DOWN]     =   'e',
    [TILE_ENEMY1_FACE_LEFT]     =   'e',
};

#define ALIGN_BUFF_TO(N) if((N) > 1) buffsize += (N) - (buffsize % (N))

static int active = 0;

static inline void* align_buff_to(int n){
    if(n) buffsize += n - (buffsize % n);
    return (void*) &buff[buffsize];
}

static inline const char* get_tile_str(char tile){
    switch (tile)
    {
    case TILE_EMPTY:
        return  "   "
                " * "
                "   ";
    case TILE_WALL:
        return  "XXX"
                "X X"
                "XXX";
    case TILE_SWALL:
        return  "###"
                "# #"
                "###";
    case TILE_PLAYER_FACE_UP:
        return  " ^ "
                " p "
                "   ";
    case TILE_PLAYER_FACE_RIGHT:
        return  "   "
                " p>"
                "   ";
    case TILE_PLAYER_FACE_DOWN:
        return  "   "
                " p "
                " V ";
    case TILE_PLAYER_FACE_LEFT:
        return  "   "
                "<p "
                "   ";
    case TILE_ENEMY1_FACE_UP:
        return  " ^ "
                " 1 "
                "   ";
    case TILE_ENEMY1_FACE_RIGHT:
        return  "   "
                " 1>"
                "   ";
    case TILE_ENEMY1_FACE_DOWN:
        return  "   "
                " 1 "
                " V ";
    case TILE_ENEMY1_FACE_LEFT:
        return  "   "
                "<1 "
                "   ";
    
    default:
        return  "~~~"
                "~~~"
                "~~~";
    }
}

static const char* get_tile_name(int tile){
    switch (tile)
    {
    case TILE_EMPTY:                return "TILE_EMPTY";
    case TILE_WALL:                 return "TILE_WALL";
    case TILE_SWALL:                return "TILE_SWALL";
    case TILE_PLAYER_FACE_UP:       return "TILE_PLAYER_FACE_UP";
    case TILE_PLAYER_FACE_RIGHT:    return "TILE_PLAYER_FACE_RIGHT";
    case TILE_PLAYER_FACE_DOWN:     return "TILE_PLAYER_FACE_DOWN";
    case TILE_PLAYER_FACE_LEFT:     return "TILE_PLAYER_FACE_LEFT";
    case TILE_ENEMY1_FACE_UP:       return "TILE_ENEMY1_FACE_UP";
    case TILE_ENEMY1_FACE_RIGHT:    return "TILE_ENEMY1_FACE_RIGHT";
    case TILE_ENEMY1_FACE_DOWN:     return "TILE_ENEMY1_FACE_DOWN";
    case TILE_ENEMY1_FACE_LEFT:     return "TILE_ENEMY1_FACE_LEFT";
    default:
        return NULL;
    }
}

static void print_map(){

    printf("\x1B[2J\x1B[H\n");

    const int irange = (cameray + camerah < maph)? cameray + camerah : maph;
    const int jrange = (camerax + cameraw < mapw)? camerax + cameraw : mapw;

    int idigit_len = 1;
    for(int _10n = 10; (int) (irange / _10n); _10n *= 10) idigit_len+=1;

    int jdigit_len = 1;
    for(int _10n = 10; (int) (jrange / _10n); _10n *= 10) jdigit_len+=1;

    for(int i = 0; i < idigit_len + 3; i+=1)
        putchar(' ');

    for(int j = camerax; j < jrange; j+=1)
        printf("%*i", jdigit_len + 1, j);
    putchar('\n');
    for(int i = 0; i < idigit_len + 3; i+=1)
        putchar(' ');
    for(int j = camerax; j < jrange; j+=1){
        for(int i = (jdigit_len / 2) + 1; i; i-=1)
            putchar(' ');
        putchar('|');
    }
    putchar('\n');
    for(int i = 0; i < idigit_len + 3; i+=1)
        putchar(' ');
    for(int j = camerax; j < jrange; j+=1)
        for(int i = 0; i < (jdigit_len + 1); i+=1)
            putchar('_');

    putchar('\n');

    for(int i = cameray; i < irange; i+=1){
        printf("%*i- |", idigit_len, i);
        for(int j = camerax; j < jrange; j+=1){
            for(int i = (jdigit_len / 2) + 1; i; i-=1)
                putchar(' ');
            putchar(
                (map[i * mapw + j] <= (sizeof(palette) / sizeof(palette[0])) && map[i * mapw + j] >= 0)?
                    (int) palette[map[i * mapw + j]] : '~'
            );
        }
        putchar('\n');
    }

}

static inline int cmp_str(const char* str1, const char* str2){
    if(!str1 || !str2) return 0;
    for(; *str1 && *str1 == *str2; str1+=1) str2 += 1;
    return *str1 == *str2;
}

static int parse_uint(const char* str){
    int output = 0;
    while (*str >= '0' && *str <= '9')
    {
        output = (output * 10) + *str++ - '0';
    }
    return (*str == '\0')? output : -1;
}

int save_map(const char* path){

    if(!path){
        fprintf(stderr, "[ERROR] missing path, required for first save\n");
        return 1;
    }

    FILE* f = fopen(path, "wb");

    if(!f){
        fprintf(stderr, "[ERROR] Could not open '%s'\n", path);
        return 1;
    }
    fprintf(f, "map:\n");
    fprintf(f, "width: %i\nheight: %i\n\n", mapw, maph);

    for(int i = 0; i < maph; i+=1){
        fputc(' ', f);
        fputc(' ', f);
        fputc(' ', f);
        for(int j = 0; j < mapw; j+=1){
            fprintf(f, " %3u,", (unsigned int) map[i * mapw + j]);
        }
        fputc('\n', f);
    }
    int i = 0;
    for(; path[i]; i+=1);
    if(i + 1 >= MAP_PATH_CAP){
        fprintf(stderr, "[WARNING] can't save new path to default, path is too long...\n");
        fclose(f);
        return 1;
    }
    map_path = map_path_buff;
    for(map_path[++i] = '\0'; i > -1; i-=1) map_path[i] = path[i];

    fclose(f);
    return 0;
}

// \returns the size of the word
static int get_next_word(const char** output, const char* str){
    char c = *str;
    for(; c == ' ' || c == '\t' || c == '\n'; c = *(++str));

    if(*str == '\0'){
        return 0;
    }

    if(output) *output = str;
    
    int word_size = 0;
    for(c = str[++word_size]; c != '\0' && c != ' ' && c != '\t' && c != '\n' && c != '\\'; c = str[++word_size]);

    return word_size;
}

static inline int fexpect(FILE* f, const char* expected, int* first_last){
    int c = *first_last;
    for(; *expected && c == *expected; c = fgetc(f)) expected+=1;
    *first_last = c;
    return *expected == '\0';
}

static inline int fparse_uint(FILE* f, int* first_last){
    int output = 0;
    int c = *first_last;
    if(c < '0' || c > '9') return -1;
    for(; c >= '0' && c <= '9'; c = fgetc(f)){
        output = (output * 10) + (c - '0');
    }
    *first_last = c;
    return output;
}

int load_map(const char* path){

    if(!path){
        fprintf(stderr, "[ERROR] missing path, required for first load\n");
    }

    FILE* f = fopen(path, "r");

    int err = 0;

    if(!f){
        fprintf(stderr, "[ERROR] Could not open '%s'\n", path);
        return 1;
    }

    int c = fgetc(f);

    for(; c == ' ' || c == '\t' || c == '\n'; c = fgetc(f));

    if(0 == fexpect(f, "map:", &c)){
        err = 1;
        fprintf(stderr, "expected 'map:' identifier\n");
        goto defer;
    }

    for(; c == ' ' || c == '\t' || c == '\n'; c = fgetc(f));
    
    if(0 == fexpect(f, "width:", &c)){
        err = 1;
        fprintf(stderr, "expected 'width:' identifier\n");
        goto defer;
    }
    for(; c == ' ' || c == '\t'; c = fgetc(f));

    const int width = fparse_uint(f, &c);
    if(width <= 0){
        fprintf(stderr, "[ERROR] invalid width\n");
        err = 1;
        goto defer;
    }
    for(; c == ' ' || c == '\t' || c == '\n'; c = fgetc(f));
    if(!fexpect(f, "height:", &c)){
        fprintf(f, "[ERROR] expected 'height:' identifier\n");
        err = 1;
        goto defer;
    }
    for(; c == ' ' || c == '\t'; c = fgetc(f));

    const int height = fparse_uint(f, &c);
    if(height <= 0){
        fprintf(stderr, "[ERROR] invalid height\n");
        err = 1;
        goto defer;
    }
    for(; c == ' ' || c == '\t' || c == '\n'; c = fgetc(f));

    int _map_size = width * height;
    for(int i = 0; i < _map_size; i+=1){
        for(; c == ' ' || c == '\t' || c == '\n'; c = fgetc(f));
        const int tile = fparse_uint(f, &c);
        if(tile < 0){
            fprintf(stderr, "[ERROR] invalid/missing tile identifier for (%i, %i)\n", i % width, (int) (i / width));
        }
        if((char) (tile) != tile){
            fprintf(stderr, "[ERROR] tile value overflow at (%i, %i)\n", i % width, (int) (i / width));
        }
        map2[i] = (char) tile;
        for(; c == ' ' || c == '\t' || c == '\n'; c = fgetc(f));
        if(c != ','){
            fprintf(stderr, "[ERROR] expected ',' after tile at (%i, %i)\n", i % width, (int) (i / width));
            err = 1;
            goto defer;
        }
        c = fgetc(f);
    }
    for(; c == ' ' || c == '\t' || c == '\n'; c = fgetc(f));
    if(c != EOF){
        fprintf(stderr, "[ERROR] Unexpected things at the end of file\n");
        fputc('\'', stderr);
        for(int i = 0; c != EOF && i < 10; i += 1){
            fputc(c, stderr);
            c = fgetc(f);
        }
        fprintf(stderr, "\'...\n");
        err = 1;
        goto defer;
    }

    for(int i = 0; i < _map_size; i+=1) map[i] = map2[i];
    mapw = width;
    maph = height;

    int i = 0;
    for(; path[i]; i+=1);
    if(i + 1 >= MAP_PATH_CAP){
        fprintf(stderr, "[WARNING] can't save new path to default, path is too long...\n");
        fclose(f);
        return 1;
    }
    map_path = map_path_buff;
    for(map_path[++i] = '\0'; i > -1; i-=1) map_path[i] = path[i];

    defer:
    fclose(f);
    return err;
}

void help(const char* what){

    printf("%s:\n", what);
    if(cmp_str(what, "exit") || cmp_str(what, "quit")){
        printf(
            "\texits the aplication\n"
        );
    }
    else if(cmp_str(what, "help")){
        printf("\tdisplays a help message\n");
    }
    else if(cmp_str(what, "place")){
        printf(
            "\tplaces a square of tile at (x, y) with pencil's width and height (check pencil instruction)\n"
            "\texpects 3 uint numbers, the tile id to place, the x and y coordinates for placement\n"
        );
    }
    else if(cmp_str(what, "pencil")){
        printf("'%s'\n", what);
        printf(
            "\tsets the width and height of the square of tiles that will be when placing or coping tiles (check place and copy)\n"
            "\texpects 2 uint numbers, width (w) and height (h)\n"
        );
    }
    else if(cmp_str(what, "move")){
        printf(
            "moves the camera to the given (x, y) position\n"
            "expects 2 uint numbers, x and y\n"
        );
    }
    else if(cmp_str(what, "zoom")){
        printf(
            "\tsets the camera's width and height"
            "\texpects 2 uint numbers, width (w) and height (h)\n"
        );
    }
    else if(cmp_str(what, "palette")){
        printf(
            "\tsets a tile's palette to the given character\n"
            "\texpects an uint number as the tile and a character as its new palette symbol\n"
        );
    }
    else if(cmp_str(what, "copy")){
        printf(
            "\tsets the position of the rect used to take a copy of a selection, width and height are given by the pencil\n"
            "\texpects 2 uint numbers, width (w) and height (h)\n"
        );
    }
    else if(cmp_str(what, "paste")){
        printf(
            "\tpastes a selection taken with copy instruction to (x, y)\n"
            "\texpects 2 uint numbers, x and y\n"
        );
    }
    else if(cmp_str(what, "check")){
        printf(
            "\tcheck the tile at (x, y)\n"
            "\texpects 2 uint numbers, x and y\n"
        );
    }
    else if(cmp_str(what, "new")){
        printf(
            "\tcreates a new map\n"
            "\texpects 2 uint numbers, width (w) and height (h)\n"
        );
    }
    else if(cmp_str(what, "save")){
        printf(
            "\tsaves the map\n"
            "\tyou can pass the path to the output save file, otherwise the map will be saved to the last loaded map or map.bin if no map was loaded\n"
        );
    }
    else if(cmp_str(what, "load")){
        printf(
            "\tloads a new map\n"
            "\tyou can pass the path to the input file, otherwise the map will be reloaded to the last loaded map\n"
        );
    }
    else{
        printf("\tno instruction named '%s'\n", what);
    }
}

#define EXPECT_ARGC(inst, argc, expected) if((argc) != (expected)){\
            fprintf(stderr, "[ERROR] " #inst " expects %i arguments, got %i instead\n", (expected), (argc));\
            return 1;\
        }
#define GET_UINT(inst, output, argv, index)\
        int output = parse_uint((argv)[(index)]);\
        if((output) < 0){\
            fprintf(stderr, "[ERROR] " #inst " %ith argument '" #output "' expects uint, got '%s' instead\n", (int) (index), (argv)[(index)]);\
            return 1;\
        }

int handle_prompt(int argc, const char** argv){

    if(argc < 1){
        print_map();
    }
    else if(cmp_str(argv[0], "help")){
        if(argc > 1){
            for(int i = 1; i < argc; i+=1) help(argv[i]);
        }
        else{
            printf(
                "available instructions are:\n"
                "\texit: quits the aplication\n"
                "\tplace <x> <y>: places a tile at (x, y)\n"
                "\tpencil <w> <h>: sets the width and height of the square for tile placement/copying\n"
                "\tmove <x> <y>: moves the camera\n"
                "\tzoom <w> <h>: sets the camera's width and height\n"
                "\tpalette <tile> <char>: sets the new symbol for displaing a tile\n"
                "\tcopy <x> <y>: sets the position of the rect used to take a copy of a selection, width and height are given by pencil\n"
                "\tpaste <x> <x>: pastes a copied selection to (x, y)\n"
                "\tcheck <x> <y>: checks the tile at (x, y)\n"
                "\tnew <w> <h>: creates a new map with (w, h) dimensions\n"
                "\tsave <optional: path>: saves the map\n"
                "\tload <optional: path>: loads a map\n"
                "\thelp <optional: what>: displays this help message or, if provided, a help message about <what>"
            );
        }
    }
    else if(cmp_str(argv[0], "exit") || cmp_str(argv[0], "quit")){
        active = 0;
    }
    else if(cmp_str(argv[0], "place")){
        EXPECT_ARGC(place, argc - 1, 3);
        GET_UINT(place, tile, argv, 1);
        GET_UINT(place, x   , argv, 2);
        GET_UINT(place, y   , argv, 3);
        const int xrange = (x + pencilw < mapw)? x + pencilw : mapw;
        const int yrange = (y + pencilh < maph)? y + pencilh : maph;
        for(int i = y; i < yrange; i+=1){
            for(int j = x; j < xrange; j+=1){
                map[i * mapw + j] = (char) tile;
            }
        }
        print_map();
    } else if(cmp_str(argv[0], "pencil")){
        EXPECT_ARGC(pencil, argc - 1, 2);
        GET_UINT(pencil, w   , argv, 1);
        GET_UINT(pencil, h   , argv, 2);
        if(w==0){
            fprintf(stderr, "[ERROR] w==0\n");
            return 1;
        }
        if(h==0){
            fprintf(stderr, "[ERROR] h==0\n");
            return 1;
        }
        pencilw = w;
        pencilh = h;
    }
    else if(cmp_str(argv[0], "move")){
        EXPECT_ARGC(move, argc - 1, 2);
        GET_UINT(move, x, argv, 1);
        GET_UINT(move, y, argv, 2);
        if(x >= mapw){
            fprintf(stderr, "[ERROR] x out of bounds\n");
            return 1;
        }
        if(y >= maph){
            fprintf(stderr, "[ERROR] y out of bound\n");
            return 1;
        }
        camerax = x;
        cameray = y;
        print_map();
    }
    else if(cmp_str(argv[0], "zoom")){
        EXPECT_ARGC(zoom, argc - 1, 2);
        GET_UINT(zoom, w, argv, 1);
        GET_UINT(zoom, h, argv, 2);
        if(w==0){
            fprintf(stderr, "[ERROR] w==0\n");
            return 1;
        }
        if(h==0){
            fprintf(stderr, "[ERROR] h==0\n");
            return 1;
        }
        cameraw = w;
        camerah = h;
        print_map();
    }
    else if(cmp_str(argv[0], "palette")){
        EXPECT_ARGC(palette, argc - 1, 2);
        GET_UINT(palette, tile_id_number, argv, 1);
        if(argv[2][1] == '\0' || argv[2][1] != '\0'){
            fprintf(stderr, "[ERROR] palette symbol should be one character long\n");
            return 1;
        }
        const char c = argv[2][0];
        for(int i = 0; i < (sizeof(palette) / sizeof(palette[0])); i += 1){
            if(palette[i] == c && i != tile_id_number){
                fprintf(stderr, "[ERROR] '%c' is already used as palette symbol for tile %i\n", c, i);
                return 1;
            }
        }
        palette[tile_id_number] = c;
        print_map();
    }
    else if(cmp_str(argv[0], "copy")){
        EXPECT_ARGC(copy, argc - 1, 2);
        GET_UINT(copy, x, argv, 1);
        GET_UINT(copy, y, argv, 2);
        copyx = x;
        copyy = y;
    }
    else if(cmp_str(argv[0], "paste")){
        EXPECT_ARGC(paste, argc - 1, 2);
        GET_UINT(paste, destx, argv, 1);
        GET_UINT(paste, desty, argv, 2);
        if(copyx < 0 || copyy < 0){
            fprintf(stderr, "No valid copied selection\n");
            return 1;
        }
        const int srcxrange = (copyx + pencilw < mapw)? pencilw : mapw;
        const int srcyrange = (copyy + pencilh < maph)? pencilh : maph;

        const int destxrange = (destx + pencilw < mapw)? pencilw : mapw;
        const int destyrange = (desty + pencilh < maph)? pencilh : maph;

        const int xrange = (srcxrange < destxrange)? srcxrange : destxrange;
        const int yrange = (srcyrange < destyrange)? srcyrange : destyrange;

        const int overlayx = (destx >= copyx && destx < copyx + srcxrange)? copyx + srcxrange - destx : 0;
        const int overlayy = (desty >= copyy && desty < copyy + srcyrange)? copyy + srcyrange - desty : 0;

        for(int i = overlayy; i < yrange; i+=1){
            for(int j = overlayx; j < xrange; j+=1){
                map[(i + desty) * mapw + (j + destx)] = map[(i + copyy) * mapw + (j + copyx)];
            }
        }
        for(int i = 0; i < overlayy; i+=1){
            for(int j = 0; j < overlayy; j+=1){
                map[(i + desty) * mapw + (j + destx)] = map[(i + copyy) * mapw + (j + copyx)];
            }
        }
        print_map();
    }
    else if(cmp_str(argv[0], "check")){
        EXPECT_ARGC(check, argc - 1, 2);
        GET_UINT(check, x, argv, 1);
        GET_UINT(check, y, argv, 2);
        if(x >= mapw){
            fprintf(stderr, "[ERROR] x out of bounds\n");
            return 1;
        }
        if(y >= maph){
            fprintf(stderr, "[ERROR] y out of bound\n");
            return 1;
        }
        const int tile = map[y * mapw + x];
        const char* tile_name = get_tile_name(tile);
        printf(
            "tile %i at (%i, %i):\n"
            "\tname: %s\n"
            "\tsymbol: %c\n",
            tile, x, y,
            tile_name? tile_name : "no name for this tile",
            (tile >= 0 && tile < (sizeof(palette) / sizeof(palette[0])))? palette[tile] : '~'
        );
        const char* tile_rep = get_tile_str(tile);
        if(!tile_rep){
            printf("\trepresentation: no representation\n");
        }
        else{
            printf("\trepresentation:\n\t\t");
            for(int i = 0; tile_rep[i]; i+=1){
                if(!(i % TILE_SIZE)){
                    putchar('\n');
                    putchar('\t');
                    putchar('\t');
                }
                putchar(tile_rep[i]);
            }
            putchar('\n');
        }
    }
    else if(cmp_str(argv[0], "new")){
        EXPECT_ARGC(new, argc - 1, 2);
        GET_UINT(new, w, argv, 1);
        GET_UINT(new, h, argv, 2);
        if(w == 0 || h == 0){
            fprintf(stderr, "[ERROR] can't have zeroed dimension(s)\n");
            return 1;
        }
        if(w * h > MAP_CAP){
            fprintf(stderr, "[ERROR] map with dimensions (%i, %i) is too big, max size is such that w * h <= %i\n", w, h, MAP_CAP);
            return 1;
        }
        for(int i = 0; i < w * h; i+=1) map[i] = TILE_EMPTY;
        mapw = w;
        maph = h;
        print_map();
    }
    else if(cmp_str(argv[0], "save")){
        if(argc > 2){
            fprintf(stderr, "[ERROR] save expects up to 1 argument (outputpath), got %i instead\n", argc - 1);
            return 1;
        }
        if(save_map((argc == 2)? argv[1] : map_path)){
            fprintf(stderr, "[ERROR] Could not save to '%s'\n", (argc == 2)? argv[1] : map_path);
            return 1;
        }
    }
    else if(cmp_str(argv[0], "load")){
        if(argc > 2){
            fprintf(stderr, "[ERROR] load expects up to 1 argument (inputpath), got %i instead\n", argc - 1);
            return 1;
        }
        if(load_map((argc == 2)? argv[1] : map_path)){
            fprintf(stderr, "[ERROR] Could not load '%s'\n", (argc == 2)? argv[1] : map_path);
            return 1;
        }
        print_map();
    }
    else{
        fprintf(stderr, "[ERROR] '%s", argv[0]);
        fputc('\'', stderr);
        fputc(' ', stderr);
        fprintf(stderr, "Unknown instruction '%s'\n", argv[0]);
        return 1;
    }

    return 0;
}

static int get_user_prompt(const char*** _argv){
    char* const inbuff = &buff[buffsize];
    const int inbuff_start = buffsize;

    for(int c = fgetc(stdin); c && c != '\n' && c != EOF; c = fgetc(stdin)){
        buff[buffsize++] = c;
    }
    buff[buffsize++] = '\0';
    buff[buffsize++] = '\0';

    int argc = 0;
    char** argv = align_buff_to(sizeof(*argv));

    int word_size = get_next_word((const char**) &argv[argc], &buff[inbuff_start]);

    for(int i = inbuff_start + word_size + 1; word_size; i += word_size + 1){
        argv[argc][word_size] = '\0';
        buffsize += sizeof(*argv);
        word_size = get_next_word((const char**) &argv[++argc], &buff[i]);
    }

    if(_argv) *_argv = (const char**) argv;

    return argc;
}

int main(int argc, char** argv){

    if(argc <= 1){
        mapw = 20;
        maph = 16;
        for(int i = 0; i < maph; i+=1)
            for(int j = 0; j < mapw; j+=1)
                map[i * mapw + j] = TILE_EMPTY;
    }

    active = 1;

    while (active)
    {
        const int    buff_scope = buffsize;
        const char** prompt_argv = NULL;
        printf(">>> ");
        const int prompt_argc = get_user_prompt(&prompt_argv);
        if(handle_prompt(prompt_argc, prompt_argv)){
            fprintf(stderr, "for a help message enter help <optional: command_name>\n");
        }
        buffsize = buff_scope;
    }
    

    putchar('\n');

    return 0;
}


