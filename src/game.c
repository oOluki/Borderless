#ifndef GAME_C
#define GAME_C

#include "game.h"
#include "maps.h"
#include "entities.h"
#include "raycast.h"
#include "renderer.h"


static const char* get_button_name(int buttonid){
    switch (buttonid)
    {
    default:
    case BUTTON_NONE: return "BUTTON_NONE";
    case BUTTON_TEST: return "BUTTON_TEST";
    }
}

int load_entity(int entity_id, int map_x, int map_y){

    switch (entity_id)
    {
    case ENTITY_PLAYER:
        game.player = (Entity){.x = map_x * TILEW, .y = map_y * TILEH, .orientation = ORIENT_UP, .state = STATE_ALIVE, .update = NULL};
        return 0;
    
    case ENTITY_ENEMY1:
        if(game.entity_count + 1 > (sizeof(game.entities) / sizeof(*game.entities))){
            ERROR("entity overflow");
            return 1;
        }
        game.entities[game.entity_count++] = (Entity){
            .x           = map_x * TILEW, .y      = map_y * TILEH,
            .orientation = ORIENT_UP    , .state  = STATE_ALIVE,
            .chill       = ENEMY1_CHILL , .update = update_enemy1
        };
        return 0;
    
    default:
        VERROR("Invalid entity_id %i", entity_id);
        return 1;
    }

    return 0;
}

int load_map(const unsigned char* src, int w, int h){

    if(w < 0 || h < 0) return 1;

    game.entity_count = 0;

    Map* const dest = &game.map;

    dest->map = mapbuff;


    dest->w = w;
    dest->h = h;

    for(int i = 0; i < h; i+=1){
        for(int j = 0; j < w; j+=1){
            if(src[i * w + j] >= TILE_FIRST_ENTITY_ID && src[i * w + j] <= TILE_LAST_ENTITY_ID){
                load_entity(src[i * w + j] - TILE_FIRST_ENTITY_ID + 1, i, j);
                dest->map[i * w + j] = TILE_EMPTY;
            }
            else if(src[i * w + j] < TILE_COUNT) dest->map[i * w + j] = src[i * w + j];
            else dest->map[i * w + j] = TILE_EMPTY;
        }
    }
    return 0;
}

void draw_map(){
    if(!game.map.map) return;

    const int irange = 1 + game.camera.h / TILEH;
    const int jrange = 1 + game.camera.w / TILEW;
    for(int i = 0; i < irange; i+=1){
        for(int j = 0; j < jrange; j+=1){
            const int tile = (int) get_tile(game.map, game.camera.x / TILEW + j, game.camera.y / TILEH + i);
            if(tile)
                fill_rect(
                    game.draw_canvas,
                    j * TILEW - (game.camera.x % TILEW),
                    i * TILEH - (game.camera.y % TILEH),
                    TILEW, TILEH,
                    (tile > 31)? 0xFF0000FF : map_palette[tile]
                );
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
        load_map(map1, map1w, map1h);
        break;
    case CMD_ENTER:
        game.update = main_screen_update;
        main_screen_update(CMD_UPDATE);
        return 1;
    case CMD_PAUSE: // TODO
        return 0;
    case CMD_MOUSECLICK: // TODO
        return 0;
    case CMD_UP:{
        int dx = 0;
        int dy = 0;
        orientation_direction(game.player.orientation, &dx, &dy);
        if(!get_tile(game.map, game.player.x / TILEW + dx, game.player.y / TILEH + dy)){
            game.player.x += dx * TILEW;
            game.player.y += dy * TILEH;
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
        if(!get_tile(game.map, game.player.x / TILEW - dx, game.player.y / TILEH - dy)){
            game.player.x -= dx * TILEW;
            game.player.y -= dy * TILEH;
        }
    }
        break;
    
    default:
        return 0;
    }

    game.camera.x = (game.player.x + TILEW / 2) - game.camera.w / 2;
    game.camera.y = (game.player.y + TILEH / 2) - game.camera.h / 2;

    clear_rect(game.draw_canvas, 0, 0, game.camera.w, game.camera.h, BACKGROUND_COLOR);

    draw_map();

    bfs_from_player(game.map, (Node){game.player.x / TILEW, game.player.y / TILEH});

    copySprite(
        game.draw_canvas,
        game.player.x - game.camera.x, game.player.y - game.camera.y,
        (game.player.state == STATE_DEAD)? SPRITE_DEAD : SPRITE_ORIENTATION + game.player.orientation,
        player_palette
    );

    for(int i = 0; i < game.entity_count; ){
        if(!game.entities[i].update){
            for(int j = i + 1; j < game.entity_count; j+=1){
                game.entities[j - 1] = game.entities[j];
            }
            game.entity_count -= 1;
            continue;
        }
        if(game.entities[i].update(game.entities + i)){
            for(int j = i + 1; j < game.entity_count; j+=1){
                game.entities[j - 1] = game.entities[j];
            }
            game.entity_count -= 1;
            continue;
        }
        i+=1;
    }

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


int general_check(){

    int output = 0;

    return output;
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