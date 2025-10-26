#ifndef GAME_C
#define GAME_C

#include "game.h"
#include "maps.h"
#include "entities.h"
#include "bfs_and_raycast.h"
#include "renderer.h"


static const char* get_button_name(int buttonid){
    switch (buttonid)
    {
    default:
    case BUTTON_NONE: return "BUTTON_NONE";
    case BUTTON_TEST: return "BUTTON_TEST";
    }
}

static inline int load_entity(int type, int x, int y, int state, int orientation, int sprite){
    if(game.entity_count + 1 >= ARLEN(game.entities)){
        ERROR("could not load entity, entity overflow\n");
        return 1;
    }
    game.entities[game.entity_count++] = (Entity){.type = type, .x = x, .y = y, .state = state, .orientation = orientation, .sprite = sprite};
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
        game.player = (Entity){.type = ENTITY_PLAYER, .x = map_x * TILEW, .y = map_y * TILEH, .orientation = ORIENT_UP   , .sprite = SPRITE_UP   , .state = STATE_ALIVE};
        return MK_TILE(TILETYPE_PLAYER, 0);
    case TILE_PLAYER_FACE_RIGHT:
        game.player = (Entity){.type = ENTITY_PLAYER, .x = map_x * TILEW, .y = map_y * TILEH, .orientation = ORIENT_RIGHT, .sprite = SPRITE_RIGHT, .state = STATE_ALIVE};
        return MK_TILE(TILETYPE_PLAYER, 0);
    case TILE_PLAYER_FACE_DOWN:
        game.player = (Entity){.type = ENTITY_PLAYER, .x = map_x * TILEW, .y = map_y * TILEH, .orientation = ORIENT_DOWN , .sprite = SPRITE_DOWN , .state = STATE_ALIVE};
        return MK_TILE(TILETYPE_PLAYER, 0);
    case TILE_PLAYER_FACE_LEFT:
        game.player = (Entity){.type = ENTITY_PLAYER, .x = map_x * TILEW, .y = map_y * TILEH, .orientation = ORIENT_LEFT , .sprite = SPRITE_LEFT , .state = STATE_ALIVE};
        return MK_TILE(TILETYPE_PLAYER, 0);

    case TILE_ENEMY1_FACE_UP:
        if(load_entity(ENTITY_ENEMY1, map_x * TILEW, map_y * TILEH, STATE_ALIVE, ORIENT_UP   , SPRITE_UP)){
            return MK_TILE(TILETYPE_ERROR, 0);
        }
        return MK_TILE(TILETYPE_ENTITY, game.entity_count - 1);
    case TILE_ENEMY1_FACE_RIGHT:
        if(load_entity(ENTITY_ENEMY1, map_x * TILEW, map_y * TILEH, STATE_ALIVE, ORIENT_RIGHT, SPRITE_RIGHT)){
            return MK_TILE(TILETYPE_ERROR, 0);
        }
        return MK_TILE(TILETYPE_ENTITY, game.entity_count - 1);
    case TILE_ENEMY1_FACE_DOWN:
        if(load_entity(ENTITY_ENEMY1, map_x * TILEW, map_y * TILEH, STATE_ALIVE, ORIENT_DOWN , SPRITE_DOWN)){
            return MK_TILE(TILETYPE_ERROR, 0);
        }
        return MK_TILE(TILETYPE_ENTITY, game.entity_count - 1);
    case TILE_ENEMY1_FACE_LEFT:
        if(load_entity(ENTITY_ENEMY1, map_x * TILEW, map_y * TILEH, STATE_ALIVE, ORIENT_LEFT , SPRITE_LEFT)){
            return MK_TILE(TILETYPE_ERROR, 0);
        }
        return MK_TILE(TILETYPE_ENTITY, game.entity_count - 1);
    
    default:
        VERROR("Invalid tile %i", tile);
        return MK_TILE(TILETYPE_ERROR, 0);
    }
}

#define loadMap(map) load_map(map, map##w, map##h)

int load_map(const unsigned char* src, int w, int h){

    if(w < 0 || h < 0) return 1;

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

static const Color* get_entity_palette(int entity){
    switch (entity)
    {
    case ENTITY_PLAYER: return player_palette;
    case ENTITY_ENEMY1: return enemy1_palette;
    
    default: { VERROR("Invalid entity %i", entity); } return NULL;
    }
}

static int draw_tile(int tile, int x, int y){
    const int tile_data = TILE_DATA(tile);
    switch (TILE_TYPE(tile))
    {
    case TILETYPE_TILE:
        fill_rect(game.draw_canvas, x, y, TILEW, TILEH, map_palette[tile_data]);
        return 0;
    case TILETYPE_PLAYER:
        if(game.player.state & STATE_ALIVE) return copy_sprite(game.draw_canvas, x, y, SPRITE_ORIENTATION + game.player.orientation, player_palette);
        return copy_sprite(game.draw_canvas, x, y, SPRITE_DEAD, player_palette);
    case TILETYPE_ENTITY:
        return copy_sprite(game.draw_canvas, x, y, game.entities[tile_data].sprite, get_entity_palette(game.entities[tile_data].type));
    default:
        break;
    }
}

void draw_map(){
    if(!game.map.map) return;

    const int ioffset = game.camera.y / TILEH;
    const int joffset = game.camera.x / TILEW;
    const int irange  = 1 + game.camera.h / TILEH;
    const int jrange  = 1 + game.camera.w / TILEW;

    int randomtile = 0;

    for(int i = 0; i < irange; i+=1){
        for(int j = 0; j < jrange; j+=1){
            const Tile tile = get_tile(game.map, j + joffset, i + ioffset);
            if(draw_tile(tile, j * TILEW - (game.camera.x % TILEW), i * TILEH - (game.camera.y % TILEH))){
                VERROR("Could not draw tile %16llx", (long long) tile);
            }
        }
    }
}

int level_update(int cmd){

    switch(cmd)
    {
    case CMD_NONE:
        return 0;
    case CMD_QUIT:
        game.active = 0;
        return 1;
    case CMD_UPDATE:
        break;
    case CMD_DEBUG:
        game.debug = !game.debug;
        break;
    case CMD_RESTART:
        loadMap(map1);
        break;
    case CMD_ENTER:
        game.update = main_screen_update;
        main_screen_update(CMD_UPDATE);
        return 1;
    case CMD_PAUSE: // TODO
        return 0;
    case CMD_MOUSECLICK:{
        clear_rect(game.draw_canvas, 0, 0, game.camera.w, game.camera.h, BACKGROUND_COLOR);

        draw_map();

        int path[10];

        printf("player at (%i, %i) to mouse at (%i, %i)\n",
        game.player.x, game.player.y, game.mouse.x + game.camera.x, game.mouse.y + game.camera.y);
        const int path_size = find_path(
            path, ARLEN(path), game.map,
            game.player.x / TILEW, game.player.y / TILEH,
            (game.mouse.x + game.camera.x) / TILEW, (game.mouse.y + game.camera.y) / TILEH
        );

        if(path_size > 0)
            printf("found path of size %i\n", path_size);
        else
            printf("no path found\n");

        for(int i = 0; i < path_size; i+=1){
            fill_rect(
                game.draw_canvas,
                (path[i] % game.map.w) * TILEW - game.camera.x,
                ((int) (path[i] / game.map.w)) * TILEH - game.camera.y,
                TILEW, TILEH,
                0xAA0000BB
            );
        }

        fill_rect(
            game.draw_canvas,
            game.mouse.x - game.mouse.w / 2, game.mouse.y - game.mouse.h / 2,
            game.mouse.w, game.mouse.h,
            0xAA11AADD
        );
    }
        return 0;
    case CMD_UP:{
        int dx = 0;
        int dy = 0;
        orientation_direction(game.player.orientation, &dx, &dy);
        const Tile tile = get_tile(game.map, game.player.x / TILEW + dx, game.player.y / TILEH + dy);
        if(tile == TILE_EMPTY){
            move_tile(game.player.x / TILEW, game.player.y / TILEH, game.player.x / TILEW + dx, game.player.y / TILEH + dy);
            game.player.x += dx * TILEW;
            game.player.y += dy * TILEH;
        }
        else if(TILE_TYPE(tile) == TILETYPE_ENTITY){
            game.entities[TILE_DATA(tile)].state = STATE_DEAD;
        }
    }
        break;
    case CMD_RIGHT:
        game.player.orientation = (game.player.orientation + 1) % ORIENT_COUNT;
        break;
    case CMD_LEFT:
        game.player.orientation = (game.player.orientation + ORIENT_COUNT - 1) % ORIENT_COUNT;
        break;
    case CMD_DOWN:{
        int dx = 0;
        int dy = 0;
        orientation_direction(game.player.orientation, &dx, &dy);
        const Tile tile = get_tile(game.map, game.player.x / TILEW - dx, game.player.y / TILEH - dy);
        if(tile == TILE_EMPTY){
            move_tile(game.player.x / TILEW, game.player.y / TILEH, game.player.x / TILEW - dx, game.player.y / TILEH - dy);
            game.player.x -= dx * TILEW;
            game.player.y -= dy * TILEH;
        }
        else if(TILE_TYPE(tile) == TILETYPE_ENTITY){
            game.entities[TILE_DATA(tile)].state = STATE_DEAD;
        }
    }
        break;
    case CMD_CHEAT_REVIVE:
        game.player.state |= STATE_ALIVE;
        break;
    
    default:
        return 0;
    }

    game.camera.x = (game.player.x + TILEW / 2) - game.camera.w / 2;
    game.camera.y = (game.player.y + TILEH / 2) - game.camera.h / 2;

    clear_rect(game.draw_canvas, 0, 0, game.camera.w, game.camera.h, BACKGROUND_COLOR);

    for(int i = 0; i < game.entity_count; ){
        if(update_entity(&game.entities[i])){
            for(int j = i + 1; j < game.entity_count; j+=1){
                game.entities[j - 1] = game.entities[j];
            }
            game.entity_count -= 1;
            continue;
        }
        i+=1;
    }

    draw_map();

    fill_rect(
        game.draw_canvas,
        game.mouse.x - game.mouse.w / 2, game.mouse.y - game.mouse.h / 2,
        game.mouse.w, game.mouse.h,
        0xAA11AADD
    );

    game.debug = 0;

    return 0;
}

int main_screen_update(int cmd){

    switch(cmd)
    {
    case CMD_NONE:
        return 0;
    case CMD_QUIT:
        game.active = 0;
        return 1;
    case CMD_UPDATE:
        break;
    case CMD_ENTER:
        load_map(map1, map1w, map1h);
        game.update = level_update;
        level_update(CMD_UPDATE);
        return 1;
    
    default:
        return 0;
    }

    const Button button = {
        .buttonid = BUTTON_TEST,
        .rect.x = 37, .rect.y = 25,
        .rect.w = 25, .rect.h = 10
    };

    clear_rect(game.draw_canvas, 0, 0, game.camera.w, game.camera.h, BACKGROUND_COLOR);

    draw_rect(
        game.draw_canvas,
        (button.rect.x * game.camera.w) / 100,
        (button.rect.y * game.camera.h) / 100,
        (button.rect.w * game.camera.w) / 100,
        (button.rect.h * game.camera.h) / 100,
        0xFF159632
    );

    render_text(
        game.draw_canvas,
        (button.rect.x * game.camera.w) / 100,
        (button.rect.y * game.camera.h) / 100,
        get_button_name(button.buttonid), 0xFFFFFFFF
    );

    return 0;
}

int game_init(Pixel* draw_canvas_pixels, int w, int h, int stride){
    game.draw_canvas = create_surface(draw_canvas_pixels, w, h, stride);

    game.camera = (Rect){.x = 0, .y = 0, .w = 32 * TILEW, .h = 24 * TILEH};
    game.mouse  = (Rect){.x = 0, .y = 0, .w = TILEW / 4 , .h = TILEH / 4 };

    load_map(map1, map1w, map1h);

    game.active = 1;

    game.update = level_update;
    
    return 0;
}

#endif // =====================  END OF FILE GAME_C ===========================