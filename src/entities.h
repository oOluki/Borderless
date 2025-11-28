#ifndef ENTITIES_HEADER
#define ENTITIES_HEADER

#include "game.h"
#include "util.h"
#include "renderer.h"

int orientation_direction(int orientation, int* dx, int* dy);

int update_entity(Entity* self);


#endif // =====================  END OF FILE ENTITIES_HEADER ===========================