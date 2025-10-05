#ifndef BEGIN_HEADER
#define BEGIN_HEADER

#include <stdio.h>
#include <stdint.h>
#include <math.h>

#define TILEW 8
#define TILEH 8

#define FPS 30

#define ERROR(MSG) fprintf(stderr, "[ERROR] " __FILE__ ":%i:0: " MSG "\n", __LINE__)
#define VERROR(MSG, ...) fprintf(stderr, "[ERROR] " __FILE__ ":%i:0: " MSG "\n", __LINE__, __VA_ARGS__)

#define MIN(X, Y) ((X) < (Y))? (X) : (Y)
#define MAX(X, Y) ((X) > (Y))? (X) : (Y)

#define MAXV(X) ((1 << (sizeof(X) - 1)) + ((1 << (sizeof(X) - 1)) - 1))


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
    int state;
    int(*update)(Game*, Entity*);
} Entity;

typedef struct Map{
    int             w;
    int             h;
    unsigned char*  map;
} Map;

typedef struct Task Task;

typedef struct Task
{
    int* data;
    int(*update)(Game* game, Task* task);
} Task;

typedef struct Ray
{
    float x;
    float y;
    float r2;
    float cos;
    float sin;
} Ray;

typedef struct Game{

    int             active;

    Surface         draw_canvas;

    Rect            camera;
    Rect            mouse;

    Map             map;

    Entity          player;

    Entity          entities[100];
    int             entity_count;

    Task            tasks[100];
    int             task_count;

    void*           user_data;

    int (*update)(Game*);
    int (*user_draw)(Game*);
    int (*user_update)(Game*);

    int debug;
} Game;



float Q_rsqrt(float number) {
    const float x2 = number * 0.5;
    const float threehalfs = 1.5;

    const int i = 0x5f3759df - ((*(int*) &number) >> 1);

    float f = *(float*) &i;

    f = f * (threehalfs - (x2 * f * f));
    f = f * (threehalfs - (x2 * f * f));
    f = f * (threehalfs - (x2 * f * f));
    f = f * (threehalfs - (x2 * f * f));
    f = f * (threehalfs - (x2 * f * f));

    return f;
}

static inline int in_bounds(const Rect rect, int x, int y){
    return (x > rect.x && x < rect.x + rect.w) && (y > rect.y && y < rect.y + rect.h);
}

static inline int in_sbounds(const Surface surface, int x, int y){
    return (x > -1 && x < surface.w) && (y > -1 && y < surface.h);
}

static inline int in_mbounds(const Map map, int x, int y){
    return (x > -1 && x < map.w) && (y > -1 && y < map.h);
}

int collide_rect(const Rect rect1, const Rect rect2){
    if(in_bounds(rect1, rect2.x, rect2.y)) return 1;
    if(in_bounds(rect1, rect2.x + rect2.w, rect2.y)) return 1;
    if(in_bounds(rect1, rect2.x, rect2.y + rect2.h)) return 1;
    if(in_bounds(rect1, rect2.x + rect2.w, rect2.y + rect2.h)) return 1;
    return 0;
}

static inline int distance2(int x1, int y1, int x2, int y2){
    return (x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1);
}

static inline unsigned char get_tile(const Map map, int x, int y){
    return (x < 0 || y < 0 || x >= map.w || y >= map.h)? '\0' : map.map[y * map.w + x];
}

static const uint32_t palettes[PALETTE_COUNT][32] = {
	{0x00000000, 0xff050305, 0xff41102e, 0xff35174a, 0xff3a1f6f, 0xff430e27, 0xff10106a, 0xff524c67, 0xff2e2e8a, 0xff0f6c40, 0xff3f5188, 0xff287377, 0xff8a2e36, 0xff955e50, 0xff683e98, 0xff7504d2, 0xff735221, 0xff89931a, 0xff484861, 0xff7c698c, 0xff629627, 0xff828282, 0xff0875e2, 0xff537194, 0xff2cb41d, 0xff499263, 0xff1b363b, 0xff077283, 0xff12edbe, 0xff56e6eb, 0xffa1fcfc, 0xffd6ffff},
    {0x00000000, 0xfff5d6e8, 0xff3c2845, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0xffcc1e24, 0xff0909d7, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000},
};

static const char spalette[] = {
    ' ', '#', '=', '\'', '\'', '\'', '\'', '\'', '\'', '\'', '\'', '\'', '\'', '\'', '\'', '\'', 'p', '@', '\'', '\'', '\'', '\'', '\'', '\'', '\'', '\'', '\'', '\'', '\'', '\'', '\'', '\''
};

#endif // =====================  END OF FILE BEGIN_HEADER ===========================