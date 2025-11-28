#ifndef MAPS_C
#define MAPS_C

#include "game.h"
#include "maps.h"



static Tile mapbuff[MAXMAPSIZE];


int place_tile(Map* map, const Tile tile, int x, int y){
    const int inbounds = !(x < 0 || y < 0 || x >= map->w || y >= map->h);
    if(inbounds)
        map->map[y * map->w + x] = tile;
    return inbounds;
}

Tile get_tile(const Map map, int x, int y){
    return (x < 0 || y < 0 || x >= map.w || y >= map.h)? TILE_EMPTY : map.map[y * map.w + x];
}

void move_tile(int x, int y, int nx, int ny){
    if(x == nx && y == ny) return;
    const Tile old = get_tile(game.map, x, y);
    const int inbounds = place_tile(&game.map, old, nx, ny);
    if(TILE_TYPE(old) == TILETYPE_ENTITY && inbounds){
        game.entities[TILE_DATA(old)].x = nx * TILEW;
        game.entities[TILE_DATA(old)].y = ny * TILEH;
    }
    else if(TILE_TYPE(old) == TILETYPE_PLAYER && inbounds){
        game.player.x = nx * TILEW;
        game.player.y = ny * TILEH;
    }
    place_tile(&game.map, TILE_EMPTY, x, y);
}


static inline int load_entity(int type, int x, int y, int state, int orientation, int item){
    if(game.entity_count + 1 >= ARLEN(game.entities)){
        ERROR("could not load entity, entity overflow\n");
        return 1;
    }
    game.entities[game.entity_count++] = (Entity){.type = type, .x = x, .y = y, .state = state, .orientation = orientation, .item = item};
    return 0;
}

static Tile load_tile_component(int tile, int map_x, int map_y){

    switch (tile)
    {
    case TILE_EMPTY:
        return MK_TILE(TILETYPE_NONE, TILE_EMPTY);
    case TILE_WALL:
        return MK_TILE(TILETYPE_TILE, TILE_WALL);
    case TILE_SWALL:
        return MK_TILE(TILETYPE_TILE, TILE_SWALL);

    case TILE_PLAYER_FACE_UP:
        game.player = (Entity){.type = ENTITY_PLAYER, .x = map_x * TILEW, .y = map_y * TILEH, .orientation = ORIENT_UP   , .state = STATE_ALIVE};
        return MK_TILE(TILETYPE_PLAYER, 0);
    case TILE_PLAYER_FACE_RIGHT:
        game.player = (Entity){.type = ENTITY_PLAYER, .x = map_x * TILEW, .y = map_y * TILEH, .orientation = ORIENT_RIGHT, .state = STATE_ALIVE};
        return MK_TILE(TILETYPE_PLAYER, 0);
    case TILE_PLAYER_FACE_DOWN:
        game.player = (Entity){.type = ENTITY_PLAYER, .x = map_x * TILEW, .y = map_y * TILEH, .orientation = ORIENT_DOWN , .state = STATE_ALIVE};
        return MK_TILE(TILETYPE_PLAYER, 0);
    case TILE_PLAYER_FACE_LEFT:
        game.player = (Entity){.type = ENTITY_PLAYER, .x = map_x * TILEW, .y = map_y * TILEH, .orientation = ORIENT_LEFT , .state = STATE_ALIVE};
        return MK_TILE(TILETYPE_PLAYER, 0);

    case TILE_ENEMY1_FACE_UP:
        if(load_entity(ENTITY_ENEMY1, map_x * TILEW, map_y * TILEH, STATE_ALIVE, ORIENT_UP   , 0)){
            return MK_TILE(TILETYPE_ERROR, 0);
        }
        return MK_TILE(TILETYPE_ENTITY, game.entity_count - 1);
    case TILE_ENEMY1_FACE_RIGHT:
        if(load_entity(ENTITY_ENEMY1, map_x * TILEW, map_y * TILEH, STATE_ALIVE, ORIENT_RIGHT, 0)){
            return MK_TILE(TILETYPE_ERROR, 0);
        }
        return MK_TILE(TILETYPE_ENTITY, game.entity_count - 1);
    case TILE_ENEMY1_FACE_DOWN:
        if(load_entity(ENTITY_ENEMY1, map_x * TILEW, map_y * TILEH, STATE_ALIVE, ORIENT_DOWN , 0)){
            return MK_TILE(TILETYPE_ERROR, 0);
        }
        return MK_TILE(TILETYPE_ENTITY, game.entity_count - 1);
    case TILE_ENEMY1_FACE_LEFT:
        if(load_entity(ENTITY_ENEMY1, map_x * TILEW, map_y * TILEH, STATE_ALIVE, ORIENT_LEFT , 0)){
            return MK_TILE(TILETYPE_ERROR, 0);
        }
        return MK_TILE(TILETYPE_ENTITY, game.entity_count - 1);
    
    default:
        VERROR("Invalid tile %i", tile);
        return MK_TILE(TILETYPE_ERROR, 0);
    }
}


int load_map(const unsigned char* src, int w, int h){

    if(w < 0 || h < 0) return 1;

    // unload map
    if(!src){
        game.map.map = NULL;
        game.map.w = 0;
        game.map.h = 0;
        return 0;
    }

    game.entity_count = 0;

    Map* const dest = &game.map;

    dest->map = mapbuff;


    dest->w = w;
    dest->h = h;

    for(int i = 0; i < h; i+=1){
        for(int j = 0; j < w; j+=1){
            dest->map[i * dest->w + j] = load_tile_component(src[i * w + j], j, i);
        }
    }
    return 0;
}

void move_tile(int x, int y, int nx, int ny);

#endif // =====================  END OF FILE MAPS_C ===========================