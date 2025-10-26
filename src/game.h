#ifndef GAME_H
#define GAME_H

#include "begin.h"


void draw_map();

int level_update(int cmd);

int main_screen_update(int cmd);

int load_map(const unsigned char* src, int w, int h);

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

int rng(int optional_seed){
    static int seed = 0;
    return (int) (&seed - &optional_seed) + seed++ - optional_seed;
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

static inline Tile get_tile(const Map map, int x, int y){
    return (x < 0 || y < 0 || x >= map.w || y >= map.h)? 0 : map.map[y * map.w + x];
}


static Game game;

#endif // =====================  END OF FILE GAME_H ===========================
