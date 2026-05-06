#ifndef ENTITIES_HEADER
#define ENTITIES_HEADER

#include "game.h"
#include "util.h"
#include "renderer.h"

int get_weapon_range(int weapon);

int fire_weapon(int weapon_range, int mapx, int mapy, int direction);

int orientation_direction(int orientation, int* dx, int* dy);

int update_entity(Entity* self);


#endif // =====================  END OF FILE ENTITIES_HEADER ===========================