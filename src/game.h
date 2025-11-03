#ifndef GAME_H
#define GAME_H

#include "begin.h"


void draw();

int level_update(int cmd);

int main_screen_update(int cmd);

int load_map(const unsigned char* src, int w, int h);

float Q_rsqrt(float number) {
    const float x2 = number * 0.5f;
    const float threehalfs = 1.5f;

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

static char get_cmd_char(int cmd){
    switch (cmd)
    {
    case CMD_NONE:              return ' ';
    case CMD_QUIT:              return 'q';
    case CMD_UPDATE:            return 'u';
    case CMD_DISPLAY:           return 'R';
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
    case 'q':   return CMD_QUIT;
    case 'R':   return CMD_DISPLAY;
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


static Game game;

#endif // =====================  END OF FILE GAME_H ===========================
