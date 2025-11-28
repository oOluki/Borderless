#ifndef GAME_H
#define GAME_H

#include "begin.h"

int rng(int optional_seed);

void draw();

int level_update(int cmd);

int option_select_update(int cmd);

const char* get_option_str(int option);

int _load_options(const int* options);

#define loadOptions(...) _load_options((int[]){__VA_ARGS__, OPTION_NONE})

// \returns the number of options
int interact_with(int* output, const Tile _tile);

int choose_option(int option, void* context, int* response);

int game_init(Pixel* draw_canvas_pixels, int draw_canvas_w, int draw_canvas_h);

extern Game game;

#endif // =====================  END OF FILE GAME_H ===========================
