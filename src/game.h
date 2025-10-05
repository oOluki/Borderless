#ifndef GAME_HEADER
#define GAME_HEADER

#include "begin.h"
#include "raycast.h"
#include "maps.h"
#include "entities.h"

int load_entity(Game* game, int entity_id, int map_x, int map_y){

    switch (entity_id)
    {
    case ENTITY_PLAYER:
        game->player = (Entity){.x = map_x * TILEW, .y = map_y * TILEH, .state = 0, .update = NULL};
        return 0;
    
    case ENTITY_ENEMY1:
        if(game->entity_count + 1 > (sizeof(game->entities) / sizeof(*game->entities))){
            ERROR("entity overflow");
            return 1;
        }
        game->entities[game->entity_count++] = (Entity){.x = map_x * TILEW, .y = map_y * TILEH, .state = 0, .update = update_enemy1};
        return 0;
    
    default:
        VERROR("Invalid entity_id %i", entity_id);
        return 1;
    }

    return 0;
}

int load_map(Game* game, const unsigned char* src, int w, int h){

    if(w < 0 || h < 0) return 1;

    Map* const dest = &game->map;

    dest->map = mapbuff;


    dest->w = w;
    dest->h = h;

    for(int i = 0; i < h; i+=1){
        for(int j = 0; j < w; j+=1){
            if(src[i * w + j] >= TILE_FIRST_ENTITY_ID && src[i * w + j] <= TILE_LAST_ENTITY_ID){
                dest->map[i * w + j] = TILE_EMPTY;
                load_entity(game, src[i * w + j] - TILE_FIRST_ENTITY_ID + 1, i, j);
            }
            else if(src[i * w + j] < TILE_COUNT) dest->map[i * w + j] = src[i * w + j];
            else dest->map[i * w + j] = TILE_EMPTY;
        }
    }
    return 0;
}


#endif // =====================  END OF FILE GAME_HEADER ===========================