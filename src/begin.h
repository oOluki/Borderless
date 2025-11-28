#ifndef BEGIN_HEADER
#define BEGIN_HEADER

#include <stdio.h>
#include <stdint.h>
#include <math.h>

#ifndef DONT_SUPPORT_SDL
    #define SUPPORT_SDL 1
#endif

#define GAME_NAME "Borderless"

#define ERROR(MSG) fprintf(stderr, "[ERROR] " __FILE__ ":%i:0: " MSG "\n", __LINE__)
#define VERROR(MSG, ...) fprintf(stderr, "[ERROR] " __FILE__ ":%i:0: " MSG "\n", __LINE__, __VA_ARGS__)

#define ASSERT(CONDITION, RETURN_VALUE) if(!(CONDITION)){\
    VERROR("ASSERTION '" #CONDITION "' FAILED");\
    return RETURN_VALUE;\
    }

#define MIN(X, Y) (((X) < (Y))? (X) :  (Y))
#define MAX(X, Y) (((X) > (Y))? (X) :  (Y))
#define ABS(X)    (((X) > 0)  ? (X) : -(X))

// array length
#define ARLEN(ARRAY) (sizeof(ARRAY) / sizeof(ARRAY[0]))

#define MAXV(X) ((1 << (sizeof(X) - 1)) + ((1 << (sizeof(X) - 1)) - 1))

#define ENEMY1_CHILL 2

#define Color uint32_t

#define BACKGROUND_COLOR 0xFF999999

#define TILEW 16
#define TILEH 16

#define FPS 30
#define FDT (1000 / FPS)


// the bit mask where a tile's type is stored in a tile
#define TILE_TYPE_MASK 7
// the bit depth where a tile's type is stored in a tile
#define TILE_TYPE_BITDEPTH 0

// the bit mask where a tile's data is stored
#define TILE_DATA_MASK (0xFF & ~TILE_TYPE_MASK)
// the bit depth where a tile's data is stored in a tile
#define TILE_DATA_BITDEPTH 3

#define TILE_TYPE(TILE) (((TILE) & TILE_TYPE_MASK) >> TILE_TYPE_BITDEPTH)

#define TILE_DATA(TILE) (((TILE) & TILE_DATA_MASK) >> TILE_DATA_BITDEPTH)

#define MK_TILE(TILE_TYPE, TILE_DATA) ((Tile) ((((TILE_TYPE) << TILE_TYPE_BITDEPTH) & TILE_TYPE_MASK) | (((TILE_DATA) << TILE_DATA_BITDEPTH) & TILE_DATA_MASK)))

enum DrawModes{
    DRAW_MODE_NONE = 0,
    DRAW_MODE_GRAPHIC,
    DRAW_MODE_CONSOLE,

    // for counting purposes
    DRAW_MODE_COUNT
};

enum Tiles{
    TILE_EMPTY = 0,
    TILE_FIRST_TILE_ID = TILE_EMPTY,
    TILE_WALL,
    TILE_SWALL,

    TILE_LAST_TILE = 15,
    TILE_PLAYER_FACE_UP,
    TILE_PLAYER_FACE_RIGHT,
    TILE_PLAYER_FACE_DOWN,
    TILE_PLAYER_FACE_LEFT,

    TILE_ENEMY1_FACE_UP,
    TILE_ENEMY1_FACE_RIGHT,
    TILE_ENEMY1_FACE_DOWN,
    TILE_ENEMY1_FACE_LEFT,

    // for counting purposes
    TILE_COUNT = 32
};

// an id number placed in the first 3 bits of a tile to id its type on the fly
// 
enum TileTypes{
    // empty tile
    TILETYPE_NONE = 0,
    // tile is an simple tile, and this tile in the Tiles enum is given by the next 5 bits of the tile
    TILETYPE_TILE,
    // tile is the player
    TILETYPE_PLAYER,
    // tile is an entity, and this entity's position in the entities array is given by the next 5 bits of the tile
    TILETYPE_ENTITY,
    // this tile is a part of a button part of id given by the next 5 bits of the tile
    TILETYPE_BUTTON,

    // for counting purposes
    TILETYPE_COUNT,

    TILETYPE_ERROR
};

enum Entities{
    ENTITY_NONE = 0,
    ENTITY_PLAYER,
    ENTITY_ENEMY1,

    // for counting purposes
    ENTITY_COUNT,
};

enum EntityStates{
    STATE_DEAD    = 0,
    STATE_ALIVE   = 1 << 0,
    STATE_DAZZLED = 1 << 1,
    STATE_ALERTED = 1 << 2,
    STATE_CARRING = 1 << 3
};

enum ORIENTATIONS{
    ORIENT_UP = 0,
    ORIENT_RIGHT,
    ORIENT_DOWN,
    ORIENT_LEFT,

    // for counting purposes
    ORIENT_COUNT
};

enum Options{
    OPTION_NONE = 0,
    OPTION_CANCEL,
    OPTION_QUIT,
    OPTION_PLAY,

    OPTION_YES,
    OPTION_NO,

    OPTION_ATTACK,
    OPTION_PUSH,
    OPTION_GRAB,
    OPTION_LOOT,

    // for counting purposes
    OPTION_COUNT
};

typedef Color Pixel;

// in game tile
typedef uint32_t Tile;

// tile used for loading maps
typedef unsigned char SrcTile;

typedef struct Rect{
    int x;
    int y;
    int w;
    int h;
} Rect;

typedef struct Surface
{
    Pixel* pixels;
    int    w;
    int    h;
    int    stride;
} Surface;

typedef struct Entity
{
    int type;
    int x;
    int y;
    int orientation;
    int state;
    int chill;
    int targetx;
    int targety;
    int item;
} Entity;

typedef struct Map{
    int     w;
    int     h;
    Tile*   map;
} Map;

typedef struct Node {
    int parent;
    int x;
    int y;
} Node;

typedef struct Ray
{
    float x;
    float y;
    float r;
    float cos;
    float sin;
} Ray;

typedef struct Game{

    int             active;

    uint8_t         draw_mode;

    Surface         draw_canvas;

    Rect            camera;
    Rect            mouse;
    //Pixel           mouse_canvas[TILEW * TILEH];

    Map             map;

    Entity          player;

    Entity          entities[32];
    uint8_t         entity_count;

    int             options[10];
    uint8_t         option_count;
    uint8_t         selected_option;

    void*           user_data;

    int (*update)(int cmd);

    int (*init_subsystem)();
    int (*close_subsystem)();
    int (*update_subsystem)();
    int (*get_cmd)();

    char            tmp_str[64];
    int             tmp_str_size;

    int debug;
} Game;


#endif // =====================  END OF FILE BEGIN_HEADER ===========================
