#ifndef GAME_C
#define GAME_C

#include "game.h"
#include "maps.h"
#include "entities.h"
#include "util.h"
#include "renderer.h"
#include "input.h"
#include "assets.h"

Game game = {};


int _load_options(const int* options){
    game.option_count = 0;
    for(int i = 0; options[i] != OPTION_NONE; i+=1){
        game.options[game.option_count++] = options[i];
    }
    return 0;
}

int rng(int optional_seed){
    static int seed = 0;
    seed = (int) ((&seed - &optional_seed) & (INTMAX_MAX | INTMAX_MIN)) + seed - optional_seed;
    return seed;
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
    case CMD_DISPLAY:
        draw();
        return 0;
    case CMD_DEBUG:
        game.debug = !game.debug;
        break;
    case CMD_CHEAT_RESTART:
        loadMap(map1);
        break;
    case CMD_BACK:
        game.update = option_select_update;
        loadOptions(OPTION_QUIT, OPTION_PLAY, OPTION_CANCEL);
        option_select_update(CMD_DISPLAY);
        return 0;
    case CMD_ENTER:{
        int dx;
        int dy;
        orientation_direction(game.player.orientation, &dx, &dy);
        game.option_count = interact_with(game.options, get_tile(game.map, game.player.x / TILEW + dx, game.player.y / TILEH + dy));
        if(game.option_count > 0){
            game.update = option_select_update;
            option_select_update(CMD_DISPLAY);
        }
    }
        return !!game.option_count;
    case CMD_TOGGLE: // TODO
        return 0;
    case CMD_MOUSECLICK:{
        clear_rect(game.draw_canvas, 0, 0, game.camera.w, game.camera.h, BACKGROUND_COLOR);

        draw();

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
        const int px = game.player.x / TILEW;
        const int py = game.player.y / TILEH;
        const Tile tile = get_tile(game.map, px + dx, py + dy);
        if(tile == TILE_EMPTY){
            move_tile(px, py, px + dx, py + dy);
        }
        else if(TILE_TYPE(tile) == TILETYPE_ENTITY){
            if(game.entities[TILE_DATA(tile)].state & STATE_ALIVE){
                game.entities[TILE_DATA(tile)].state = STATE_DEAD;
            }
            else{
                const Tile t = get_tile(game.map, px + 2 * dx, py + 2 * dy);
                if(t == TILE_EMPTY){
                    move_tile(px + dx, py + dy, px + 2 * dx, py + 2 * dy);
                    move_tile(px, py, px + dx, py + dy);
                }
            }
        }
    }
        break;
    case CMD_RIGHT:
        game.player.orientation = (game.player.orientation + 1) % ORIENT_COUNT;
        game.player.state &= ~STATE_CARRING;
        break;
    case CMD_LEFT:
        game.player.orientation = (game.player.orientation + ORIENT_COUNT - 1) % ORIENT_COUNT;
        game.player.state &= ~STATE_CARRING;
        break;
    case CMD_DOWN:{
        int dx = 0;
        int dy = 0;
        orientation_direction(game.player.orientation, &dx, &dy);
        const int px = game.player.x / TILEW;
        const int py = game.player.y / TILEH;
        const Tile tile = get_tile(game.map, px - dx, py - dy);
        if(tile == TILE_EMPTY){
            move_tile(px, py, px - dx, py - dy);
            if(game.player.state & STATE_CARRING){
                const Tile t = get_tile(game.map, px + dx, py + dy);
                if(t != TILE_EMPTY){
                    move_tile(px + dx, py + dy, px, py);
                }
                else{
                    game.player.state &= ~STATE_CARRING;
                }
            }
        }
    }
        break;
    
    default:
        return 0;
    }

    game.camera.x = (game.player.x + TILEW / 2) - game.camera.w / 2;
    game.camera.y = (game.player.y + TILEH / 2) - game.camera.h / 2;

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

    draw();

    game.debug = 0;

    return 0;
}

int option_select_update(int cmd){

    switch(cmd)
    {
    case CMD_NONE:
        return 0;
    case CMD_QUIT:
        game.active = 0;
        return 1;
    case CMD_UPDATE:
    case CMD_DISPLAY:
        break;
    case CMD_DOWN:
        if(game.option_count) game.selected_option = (game.selected_option + 1) % game.option_count;
        break;
    case CMD_UP:
        if(game.option_count) game.selected_option = (game.selected_option == 0)? game.option_count - 1 : game.selected_option - 1;
        break;
    case CMD_BACK:
        game.update = level_update;
        game.option_count = 0;
        game.tmp_str_size = 0;
        level_update(CMD_DISPLAY);
        return 0;
    case CMD_MOUSECLICK:
    case CMD_ENTER:
        if(game.selected_option >= 0 && game.selected_option < game.option_count){
            if(choose_option(game.options[game.selected_option], &game.player, NULL)){
                game.update = level_update;
                game.option_count = 0;
                game.tmp_str_size = 0;
                level_update(CMD_DISPLAY);
                return 0;
            }
        }
        return 0;
    
    default:
        return 0;
    }

    draw();

    return 0;
}


int interact_with(int* output, const Tile _tile){
    const int tile = TILE_DATA(_tile);
    int count = 0;
    switch (TILE_TYPE(_tile))
    {
    case TILETYPE_ENTITY:
        output[count++] = OPTION_ATTACK;
        output[count++] = OPTION_PUSH;
        output[count++] = OPTION_GRAB;
        output[count++] = OPTION_LOOT;
        break;
    
    default:
        game.tmp_str_size += feed_str(
            &game.tmp_str[game.tmp_str_size],
            "Nothing to interact with...",
            sizeof(game.tmp_str) - game.tmp_str_size
        );
        break;
    }
    output[count++] = OPTION_CANCEL;
    return count;
}

int choose_option(int option, void* context, int* response){
    switch (option)
    {
    case OPTION_NONE:
        return 0;
    case OPTION_QUIT:
        game.active = 0;
    case OPTION_CANCEL:
        return 1;
    case OPTION_PLAY:
        game.entity_count = 0;
        game.option_count = 0;
        loadMap(map1);
        game.update = level_update;
        level_update(CMD_DISPLAY);
        return 1;
    
    case OPTION_YES:
        if(response) *response = 1;
        return 1;
    case OPTION_NO:
        if(response) *response = 0;
        return 1;

    case OPTION_ATTACK:{
        const Entity* const entity = (Entity*) context;
        int dx;
        int dy;
        orientation_direction(entity->orientation, &dx, &dy);
        const Tile tile = get_tile(game.map, entity->x / TILEW + dx, entity-> y / TILEH + dy);
        if(TILE_TYPE(tile) != TILETYPE_ENTITY)
            return 1;
        game.entities[TILE_DATA(tile)].state = STATE_DEAD;
    }
        return 1;
    case OPTION_PUSH:{
        const Entity* const entity = (Entity*) context;
        int dx;
        int dy;
        orientation_direction(entity->orientation, &dx, &dy);
        const int px = entity->x / TILEW;
        const int py = entity->y / TILEH;
        const Tile tile = get_tile(game.map, px + dx, py + dy);
        if(TILE_TYPE(tile) != TILETYPE_ENTITY)
            return 1;
        if(get_tile(game.map, px + 2 * dx, py + 2 * dy) == TILE_EMPTY){
            move_tile(px + dx, py + dy, px + 2 * dx, py + 2 * dy);
        }
        game.entities[TILE_DATA(tile)].state |= STATE_ALERTED;
    }
        return 1;
    case OPTION_GRAB:{
        Entity* const entity = (Entity*) context;
        if(entity->state & STATE_CARRING){
            entity->state &= ~STATE_CARRING;
            return 1;
        }
        int dx;
        int dy;
        orientation_direction(entity->orientation, &dx, &dy);
        const int px = entity->x / TILEW;
        const int py = entity->y / TILEH;
        const Tile tile = get_tile(game.map, px + dx, py + dy);
        if(TILE_TYPE(tile) != TILETYPE_ENTITY)
            return 1;
        entity->state |= STATE_CARRING;
        if(game.entities[TILE_DATA(tile)].state & STATE_ALIVE)
            game.entities[TILE_DATA(tile)].state |= STATE_ALERTED;
    }
        return 1;
    case OPTION_LOOT:{
        Entity* const entity = (Entity*) context;
        int dx;
        int dy;
        orientation_direction(entity->orientation, &dx, &dy);
        const int px = entity->x / TILEW;
        const int py = entity->y / TILEH;
        const Tile tile = get_tile(game.map, px + dx, py + dy);
        if(TILE_TYPE(tile) != TILETYPE_ENTITY)
            return 1;
        if(game.entities[TILE_DATA(tile)].state & STATE_ALIVE)
            return 1;
        if(game.entities[TILE_DATA(tile)].item)
            entity->item = game.entities[TILE_DATA(tile)].item;
    }
        return 1;

    default:
        VERROR("Invalid option %i\n", option);
        return 1;
    }
}

const char* get_option_str(int option){
    switch (option)
    {
    default:
    case OPTION_NONE:       return "OPTION_NONE";
    case OPTION_CANCEL:     return "CANCEL";
    case OPTION_QUIT:       return "QUIT";
    case OPTION_PLAY:       return "PLAY";

    case OPTION_YES:        return "YES";
    case OPTION_NO:         return "NO";

    case OPTION_ATTACK:     return "ATTACK";
    case OPTION_PUSH:       return "PUSH";
    case OPTION_GRAB:       return "GRAB OR RELEASE";
    case OPTION_LOOT:       return "LOOT";
    }
}

int game_init(Pixel* draw_canvas_pixels, int draw_canvas_w, int draw_canvas_h){

    game.draw_canvas = (Surface){draw_canvas_pixels, draw_canvas_w, draw_canvas_h, draw_canvas_w};

    if(draw_canvas_w * draw_canvas_h < game.draw_canvas.w * game.draw_canvas.h || draw_canvas_w < game.draw_canvas.w){
        ERROR("draw_canvas_pixels does not have minimum required capacity\n");
        return 1;
    }

    game.camera.x = 0;
    game.camera.y = 0;

    if(game.camera.w <= 0) game.camera.w = 24 * TILEW;
    if(game.camera.h <= 0) game.camera.h = 18 * TILEH;

    game.mouse  = (Rect){.x = 0, .y = 0, .w = TILEW / 4 , .h = TILEH / 4 };

    load_map(map1, map1w, map1h);

    game.active = 1;

    game.update = level_update;
    
    return 0;
}

#endif // =====================  END OF FILE GAME_C ===========================