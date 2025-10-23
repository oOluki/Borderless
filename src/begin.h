#ifndef BEGIN_HEADER
#define BEGIN_HEADER

#include <stdio.h>
#include <stdint.h>
#include <math.h>

#define GAME_NAME "Borderless"

#define TILEW 8
#define TILEH 8

#define FPS 30
#define FDT (1000 / FPS)

#define ERROR(MSG) fprintf(stderr, "[ERROR] " __FILE__ ":%i:0: " MSG "\n", __LINE__)
#define VERROR(MSG, ...) fprintf(stderr, "[ERROR] " __FILE__ ":%i:0: " MSG "\n", __LINE__, __VA_ARGS__)

#define MIN(X, Y) (((X) < (Y))? (X) :  (Y))
#define MAX(X, Y) (((X) > (Y))? (X) :  (Y))
#define ABS(X)    (((X) > 0)  ? (X) : -(X))

#define MAXV(X) ((1 << (sizeof(X) - 1)) + ((1 << (sizeof(X) - 1)) - 1))

#define ENEMY1_CHILL 2

#define BACKGROUND_COLOR 0xFF999999

enum Palettes{
    PALETTE1 = 0,
    PALETTE_MAP,

    // for counting purposes
    PALETTE_COUNT
};

enum Tiles{
    TILE_EMPTY = 0,
    TILE_FIRST_TILE_ID = TILE_EMPTY,
    TILE_WALL,
    TILE_SWALL,

    TILE_LAST_TILE_ID = 15,

    TILE_FIRST_ENTITY_ID = 16,
    TILE_PLAYER = TILE_FIRST_ENTITY_ID,
    TILE_ENEMY1,
    TILE_ENEMY1_ALERT,
    TILE_LAST_ENTITY_ID = 31,

    // for counting purposes
    TILE_COUNT = 32
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
    STATE_ALERTED = 1 << 2
};

enum ORIENTATIONS{
    ORIENT_UP = 0,
    ORIENT_RIGHT,
    ORIENT_DOWN,
    ORIENT_LEFT,

    // for counting purposes
    ORIENT_COUNT
};

enum Cmd{
    CMD_NONE = 0,
    
    CMD_QUIT,
    CMD_UPDATE,
    CMD_DEBUG,
    CMD_RESTART,

    CMD_ENTER,
    CMD_PAUSE,
    CMD_MOUSECLICK,

    CMD_UP,
    CMD_RIGHT,
    CMD_LEFT,
    CMD_DOWN,

    // for external user usage
    CMD_SPECIAL_SIGNAL,

    // indicates last command in command queue
    CMD_FINNISHED,

    // for counting purposes
    CMD_COUNT
};

enum Buttons{
    BUTTON_NONE = 0,
    BUTTON_TEST,

    // for counting purposes
    BUTTON_COUNT
};

typedef struct Game Game;

typedef uint32_t Pixel;

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

typedef struct Entity Entity;

typedef struct Entity
{
    int x;
    int y;
    int orientation;
    int state;
    int chill;
    int targetx;
    int targety;
    int(*update)(Entity*);
} Entity;

typedef struct Map{
    int             w;
    int             h;
    unsigned char*  map;
} Map;

typedef struct Node {int x; int y;} Node;

typedef struct Task Task;

typedef struct Task
{
    int* data;
    int(*update)(Task* task);
} Task;

typedef struct Button{
    int   buttonid;
    Rect rect;
} Button;

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

    Surface         draw_canvas;

    Rect            camera;
    Rect            mouse;
    //Pixel           mouse_canvas[TILEW * TILEH];

    Map             map;

    Entity          player;

    Entity          entities[100];
    int             entity_count;

    Task            tasks[100];
    int             task_count;

    void*           user_data;

    int (*update)(int cmd);

    int debug;
} Game;


#endif // =====================  END OF FILE BEGIN_HEADER ===========================