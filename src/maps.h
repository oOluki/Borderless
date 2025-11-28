#ifndef MAPS_HEADER
#define MAPS_HEADER

#include "begin.h"

#define MAXMAPSIZE (1024)

int place_tile(Map* map, Tile tile, int x, int y);

Tile get_tile(const Map map, int x, int y);


int load_map(const unsigned char* src, int w, int h);

#define loadMap(map) load_map(map, map##w, map##h)

void move_tile(int x, int y, int nx, int ny);


#endif // =====================  END OF FILE MAPS_HEADER ===========================