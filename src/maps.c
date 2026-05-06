#ifndef MAPS_C
#define MAPS_C

#include "game.h"
#include "maps.h"
#include "assets.h"



static Tile mapbuff[MAXMAPSIZE];

const char* get_map_str(int MAP){
    switch (MAP)
    {
    case MAP_0:     return "MAP_0";
    case MAP_TEST:  return "MAP_TEST";
    
    default:
        ETODO(MAP);
        return NULL;
    }
}

int place_tile(Map* map, const Tile tile, int x, int y){
    const int inbounds = !(x < 0 || y < 0 || x >= map->w || y >= map->h);
    if(inbounds)
        map->map[y * map->w + x] = tile;
    return inbounds;
}

Tile get_tile(const Map map, int x, int y){
    return (x < 0 || y < 0 || x >= map.w || y >= map.h)? TILE_NONE : map.map[y * map.w + x];
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
    place_tile(&game.map, TILE_NONE, x, y);
}


static inline int load_entity(int type, int x, int y, int state, int orientation, int items){
    if(game.entity_count + 1 >= ARLEN(game.entities)){
        ERROR("could not load entity, entity overflow\n");
        return 1;
    }
    game.entities[game.entity_count++] = (Entity){.id = type, .x = x, .y = y, .state = state, .orientation = orientation, .items = items};
    return 0;
}

static Tile load_tile_component(int TILE, int map_x, int map_y, const unsigned char* src){

    if(TILE >= TILE_FIRST_ENTITY){

        if(game.entity_count + 1 >= ARLEN(game.entities)){
            ERROR("could not load entity, entity overflow\n");
            return MK_TILE(TILETYPE_ERROR, 0);
        }
        game.entities[game.entity_count++] = (Entity){
            .id = ENTITY_ENEMY1,
            .x = map_x * TILEW,
            .y = map_y * TILEH,
            .orientation = src[map_x * map_y + TILE - TILE_FIRST_ENTITY] >> 6,
            .state  = STATE_ALIVE,
            .weapon = src[map_x * map_y + TILE - TILE_FIRST_ENTITY] & ((1 << 6) - 1)
        };
        return MK_TILE(TILETYPE_ENTITY, game.entity_count - 1);
    }

    switch (TILE)
    {
    case TILE_NONE:
        return MK_TILE(TILETYPE_NONE, TILE_NONE);
    case TILE_WALL:
        return MK_TILE(TILETYPE_TILE, TILE_WALL);
    case TILE_SWALL:
        return MK_TILE(TILETYPE_TILE, TILE_SWALL);

    case TILE_PLAYER_FACE_UP:
    case TILE_PLAYER_FACE_RIGHT:
    case TILE_PLAYER_FACE_DOWN:
    case TILE_PLAYER_FACE_LEFT:
        game.player = (Entity){
            .id = ENTITY_PLAYER,
            .x = map_x * TILEW,
            .y = map_y * TILEH,
            .orientation = TILE - TILE_PLAYER_FACE_UP,
            .state = STATE_ALIVE
        };
        return MK_TILE(TILETYPE_PLAYER, 0);
    
    default:
        ETODO(TILE);
        return MK_TILE(TILETYPE_ERROR, 0);
    }
}


int load_map(int _map){

    if(_map < 0 || _map >= MAP_COUNT)
        ERROR("map %i overflows maps array", _map);

    const LoadMap map = maps[_map];

    if(map.w < 0 || map.h < 0) return 1;

    // unload map
    if(!map.map){
        game.map.map = NULL;
        game.map.w = 0;
        game.map.h = 0;
        return 0;
    }

    game.entity_count = 0;

    Map* const dest = &game.map;

    dest->map = mapbuff;


    dest->w = map.w;
    dest->h = map.h;

    for(int i = 0; i < map.h; i+=1){
        for(int j = 0; j < map.w; j+=1){
            dest->map[i * dest->w + j] = load_tile_component(map.map[i * map.w + j], j, i, map.map);
        }
    }

    return 0;
}

void move_tile(int x, int y, int nx, int ny);

#endif // =====================  END OF FILE MAPS_C ===========================