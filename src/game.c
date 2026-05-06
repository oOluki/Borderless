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

typedef struct _change_update_arg_t {
    int cmd;
    int dont_clean_message;
} _change_update_arg_t;

static inline int _change_update(int(*f)(int cmd), _change_update_arg_t arg){
    if(0 == arg.dont_clean_message)
        game.tmp_message_frames = 0;
    if(f == NULL)
        return 0;
    game.update = f;
    return f(arg.cmd);
}

#define change_update(NEW_UPDATE_METHOD, ...) _change_update((NEW_UPDATE_METHOD), (_change_update_arg_t){__VA_ARGS__})

int _load_options(const int* options){
    game.option_count = 0;
    for(int i = 0; options[i] != OPTION_NONE; i+=1){
        game.options[game.option_count++] = options[i];
    }
    return 0;
}

int rng(){
    static unsigned char call = 0;
    static const int rng_values[] = {
        90, 77, 13, 69, 57,
        81, 59, 19, 69, 52,
        82, 84, 76, 72, 97,
        47, 60, 98, 96, 00,
        68, 34, 89, 86, 19,
        54, 14, 56, 13, 52,
        49, 10, 78, 20, 100,
        62, 94, 15, 32, 40,
        06, 43, 18, 02, 74,
        16, 89, 38, 48, 63,
    };
    
    const unsigned int _call = call;
    call = (call + 1) % ARLEN(rng_values);
    return rng_values[_call];
}

const char* get_weapon_str(int weapon){
    switch (weapon)
    {
    case WEAPON_PISTOL: return "PISTOL";
    
    default:
        TODO("weapon %i", weapon);
    case WEAPON_NONE:
        return NULL;
    }
}

static int move_mouse_update(int cmd){

    switch (cmd)
    {
    case CMD_NONE:
        return 0;
    case CMD_QUIT:
        game.active = 0;
        return 1;
    case CMD_UP:
    case CMD_DOWN:
        game.mousey += (cmd == CMD_DOWN)? TILEH : -TILEH;
        break;
    case CMD_RIGHT:
    case CMD_LEFT:
        game.mousex += (cmd == CMD_RIGHT)? TILEW : -TILEW;
        break;
    case CMD_LCLICK:
    case CMD_ENTER:
        choose_option(game.option_callback, &game.player);
        if(game.active == 0)
            return 1;
    case CMD_BACK:
        game.update = level_update;
        game.tmp_message_frames = 0;
        return level_update(CMD_DISPLAY);;
    default:
        break;
    }

    if(game.mousey > game.camera.y + game.camera.h)
        game.mousey = game.camera.y + game.camera.h - TILEH;
    if(game.mousey < MAX(game.camera.y, 0))
        game.mousey = MAX(game.camera.y, 0);
    if(game.mousex > game.camera.x + game.camera.w)
        game.mousex = game.camera.x + game.camera.w - TILEW;
    if(game.mousex < MAX(game.camera.x, 0))
        game.mousex = MAX(game.camera.x, 0);

    const int mx = game.mousex / TILEW;
    const int my = game.mousey / TILEH;
    message(-1,
        "cursor (%i, %i)", .arg[0].i = mx, .arg[1].i = my
    );

    const Tile mouse_tile = get_tile(game.map, mx, my) | TILE_MARK_MASK;

    place_tile(&game.map, mouse_tile, mx, my);

    draw();

    return 0;
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
    case CMD_BACK:
        loadOptions(OPTION_QUIT, OPTION_PLAY, OPTION_TEST, OPTION_CANCEL);
        return change_update(option_select_update, .cmd = CMD_DISPLAY);
    case CMD_ENTER:{
        int dx;
        int dy;
        orientation_direction(game.player.orientation, &dx, &dy);
        game.option_count = interact_with(game.options, &game.player, get_tile(game.map, game.player.x / TILEW + dx, game.player.y / TILEH + dy));
        if(game.option_count > 0){
            game.update = option_select_update;
            option_select_update(CMD_DISPLAY);
        }
    }
        return !!game.option_count;
    case CMD_RCLICK: // TODO
        return 0;
    case CMD_LCLICK:{
        int path[10];

        const int path_size = find_path(
            path, ARLEN(path), game.map,
            game.player.x / TILEW, game.player.y / TILEH,
            game.mousex / TILEW, game.mousey / TILEH
        );

        for(int i = 0; i < path_size; i+=1){
            const int x = path[i] % game.map.w;
            const int y = (int) (path[i] / game.map.w);
            const Tile t = get_tile(game.map, x, y);
            place_tile(&game.map, t | TILE_MARK_MASK, x, y);
        }

        draw();
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
                game.entities[TILE_DATA(tile)].state &= ~STATE_CLEAN_ON_KILL;
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
    case CMD_LEFT:
        if(game.player.state & STATE_CARRING){
            int dx = 0;
            int dy = 0;
            orientation_direction(game.player.orientation, &dx, &dy);
            const Tile tile = get_tile(game.map, game.player.x / TILEW + dx, game.player.y / TILEH + dy);
            if(TILE_TYPE(tile) == TILETYPE_ENTITY)
                game.entities[TILE_DATA(tile)].state &= ~STATE_GRABBED;
            game.player.state &= ~STATE_CARRING;
        }
        game.player.orientation =
            (game.player.orientation + ((cmd == CMD_RIGHT)? 1 : ORIENT_COUNT - 1)) % ORIENT_COUNT;
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

    if(game.option_count <= 0){
        return change_update(level_update, .cmd = CMD_DISPLAY);
    }

    switch(cmd)
    {
    case CMD_NONE:
        return 0;
    case CMD_QUIT:
        game.active = 0;
        return 1;
    case CMD_DOWN:
        if(game.option_count) game.selected_option = (game.selected_option + 1) % game.option_count;
        break;
    case CMD_UP:
        if(game.option_count) game.selected_option = (game.selected_option == 0)? game.option_count - 1 : game.selected_option - 1;
        break;
    case CMD_BACK:
        return change_update(level_update, .cmd = CMD_DISPLAY);
    case CMD_LCLICK:
    case CMD_ENTER:
        if(game.selected_option >= 0 && game.selected_option < game.option_count){
            if(choose_option(game.options[game.selected_option], &game.player)){
                if(0 == game.active)
                    return 1;
                return change_update(level_update, .cmd = CMD_UPDATE);
            }
        }
        return 0;
    
    default:
        break;
    }

    draw();

    return 0;
}


static inline void load_map_and_center_camera(int map){
    load_map(map);
    game.camera.x = game.player.x - game.camera.w / 2;
    game.camera.y = game.player.y - game.camera.h / 2;
}

static int map_select_update(int cmd){

    switch (cmd)
    {
    case CMD_QUIT:
        game.active = 0;
        return 1;
    case CMD_BACK:
    case CMD_ENTER:
        return change_update(level_update, .cmd = CMD_DISPLAY);
    case CMD_UP:
        game.map_number = MAP_0;
        load_map_and_center_camera(game.map_number);
        break;
    case CMD_DOWN:
        game.map_number = MAP_COUNT - 1;
        DEBUG_CODE(if(game.map_number < 0) {ERROR("game.map_number < 0"); return 1;});
        load_map_and_center_camera(game.map_number);
        break;
    case CMD_RIGHT:
        DEBUG_CODE(if(game.map_number < 0) {ERROR("MAP_COUNT == 0"); return 1;});
        game.map_number = (game.map_number + 1) % MAP_COUNT;
        load_map_and_center_camera(game.map_number);
        break;
    case CMD_LEFT:
        DEBUG_CODE(if(game.map_number < 0) {ERROR("MAP_COUNT == 0"); return 1;});
        game.map_number = (game.map_number <= 0)? MAP_COUNT - 1 : game.map_number - 1;
        load_map_and_center_camera(game.map_number);
        break;
    
    default:
        break;
    }

    message(1, "map: %s", .arg[0].str = get_map_str(game.map_number));

    draw();

    return 0;
}


int interact_with(int* output, const Entity* entity, const Tile _tile){
    const int tile = TILE_DATA(_tile);
    int count = 0;

    if(entity->weapon != WEAPON_NONE)
        output[count++] = OPTION_FIRE;

    switch (TILE_TYPE(_tile))
    {
    case TILETYPE_ENTITY:
        if(game.entities[tile].state & STATE_ALIVE)
            output[count++] = OPTION_ATTACK;
        output[count++] = OPTION_PUSH;
        output[count++] = (game.entities[tile].state & STATE_GRABBED)? OPTION_RELEASE : OPTION_GRAB;
        output[count++] = OPTION_LOOT;
        break;
    
    default:
        if(entity->weapon == WEAPON_NONE) {
            message(-1, "Nothing to interact with...");
        }
        break;
    }
    output[count++] = OPTION_MOVECURSOR;
    output[count++] = OPTION_CANCEL;
    return count;
}

int choose_option(int OPTION, void* context){
    switch (OPTION)
    {
    case OPTION_NONE:
        return 0;
    case OPTION_QUIT:
        game.active = 0;
    case OPTION_CANCEL:
        change_update(level_update, .cmd = CMD_DISPLAY);
        return 0;
    case OPTION_PLAY:
        game.entity_count = 0;
        game.tmp_message_frames = 0;
        load_map(game.map_number);
        game.update = level_update;
        level_update(CMD_DISPLAY);
        return 1;
    
    case OPTION_YES:
        return choose_option(game.option_callback, context);
    case OPTION_NO:
        return 1;

    case OPTION_ATTACK:{
        const Entity* const entity = (Entity*) context;
        int dx;
        int dy;
        orientation_direction(entity->orientation, &dx, &dy);
        const Tile tile = get_tile(game.map, entity->x / TILEW + dx, entity-> y / TILEH + dy);
        if(TILE_TYPE(tile) != TILETYPE_ENTITY)
            return 1;
        game.entities[TILE_DATA(tile)].state &= ~STATE_CLEAN_ON_KILL;
    }
        return 1;
    case OPTION_PUSH:{
        Entity* const entity = (Entity*) context;
        int dx;
        int dy;
        orientation_direction(entity->orientation, &dx, &dy);
        const int px = entity->x / TILEW;
        const int py = entity->y / TILEH;
        const Tile tile = get_tile(game.map, px + dx, py + dy);
        if(TILE_TYPE(tile) == TILETYPE_ENTITY){
            if(get_tile(game.map, px + 2 * dx, py + 2 * dy) == TILE_EMPTY){
                move_tile(px + dx, py + dy, px + 2 * dx, py + 2 * dy);
            }
            game.entities[TILE_DATA(tile)].state &= ~STATE_GRABBED;
            entity->state &= ~STATE_CARRING;
            if(game.entities[TILE_DATA(tile)].state & STATE_ALIVE)
                game.entities[TILE_DATA(tile)].state |= STATE_ALERTED;
        }
    }
        return 1;
    case OPTION_GRAB:{
        Entity* const entity = (Entity*) context;
        int dx;
        int dy;
        orientation_direction(entity->orientation, &dx, &dy);
        const int px = entity->x / TILEW;
        const int py = entity->y / TILEH;
        const Tile tile = get_tile(game.map, px + dx, py + dy);
        if(TILE_TYPE(tile) == TILETYPE_ENTITY){
            game.entities[TILE_DATA(tile)].state |= STATE_GRABBED;
            if(game.entities[TILE_DATA(tile)].state & STATE_ALIVE)
                game.entities[TILE_DATA(tile)].state |= STATE_ALERTED;
        }
        entity->state |= STATE_CARRING;
    }
        return 1;
    case OPTION_RELEASE:{
        Entity* const entity = (Entity*) context;
        int dx;
        int dy;
        orientation_direction(entity->orientation, &dx, &dy);
        const int px = entity->x / TILEW;
        const int py = entity->y / TILEH;
        const Tile tile = get_tile(game.map, px + dx, py + dy);
        if(TILE_TYPE(tile) == TILETYPE_ENTITY){
            game.entities[TILE_DATA(tile)].state &= ~STATE_GRABBED;
        }
        entity->state &= ~STATE_CARRING;
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
        Entity* const other = &game.entities[TILE_DATA(tile)];
        if(other->state & STATE_ALIVE){
            other->state |= STATE_ALERTED;
            const int robbed_item = rng() & ITEM_FULL_MASK;
            other->items &= ~robbed_item;
            entity->items |= robbed_item;
            return 1;
        }
        if(entity->id == ENTITY_PLAYER && entity->weapon != WEAPON_NONE && other->weapon != WEAPON_NONE && other->weapon != entity->weapon){
            if(game.option_callback){
                entity->weapon = other->weapon;
                other->weapon = WEAPON_NONE;
                game.option_callback = OPTION_NONE;
            }
            else{
                message(-1, "want to swap your %s for %s?",
                    .arg[0].str = get_weapon_str(entity->weapon), .arg[1].str = get_weapon_str(other->weapon)
                );
                loadOptions(OPTION_YES, OPTION_NO);
                game.option_callback = OPTION;
                return 0;
            }
        }
        else if(entity->weapon == WEAPON_NONE){
            entity->weapon = other->weapon;
            other->weapon = WEAPON_NONE;
        }
        entity->items |= other->items;
        other->items = ITEM_NONE;
    }
        return 1;
    case OPTION_FIRE:{
        const Entity* entity = (const Entity*) context;
        if(entity->weapon != WEAPON_NONE)
            fire_weapon(get_weapon_range(entity->weapon), entity->x / TILEW, entity->y / TILEH, entity->orientation);
    }
        return 1;
    case OPTION_MOVECURSOR:
        return change_update(move_mouse_update, .cmd = CMD_DISPLAY);
    case OPTION_TEST:
        loadOptions(OPTION_LOAD_MAP, OPTION_REVIVE, OPTION_TEST_RNG, OPTION_CANCEL);
        option_select_update(CMD_DISPLAY);
        return 0;
    case OPTION_LOAD_MAP:
        return change_update(map_select_update, .cmd = CMD_DISPLAY);
    case OPTION_TEST_RNG:
        message(-1, "rng: %i", .arg[0].i = rng());
        option_select_update(CMD_DISPLAY);
        return 0;
    case OPTION_REVIVE:{
        game.tmp_message_frames = 0;
        const Tile tile = get_tile(game.map, game.mousex / TILEW, game.mousey / TILEH);
        if(TILE_TYPE(tile) == TILETYPE_ENTITY){
            game.entities[TILE_DATA(tile)].state &= ~STATE_CLEAN_ON_KILL;
            game.entities[TILE_DATA(tile)].state |= STATE_ALIVE;
        }
        else if(TILE_TYPE(tile) == TILETYPE_PLAYER){
            game.player.state &= ~STATE_CLEAN_ON_KILL;
            game.player.state |= STATE_ALIVE;
        }
        else{
            message(1, "nothing to revive at (x, y) = (%i, %i)", .arg[0].i = game.mousex / TILEW, .arg[1].i = game.mousey / TILEW);
            return change_update(level_update, .cmd = CMD_DISPLAY, .dont_clean_message = 1);
        }
        game.update = level_update;
        level_update(CMD_DISPLAY);
    }
        return 0;

    default:
        ETODO(OPTION);
        return 1;
    }
}

const char* get_option_str(int OPTION){
    switch (OPTION)
    {
    case OPTION_NONE:       return "OPTION_NONE";
    case OPTION_CANCEL:     return "CANCEL";
    case OPTION_QUIT:       return "QUIT";
    case OPTION_PLAY:       return "PLAY";

    case OPTION_YES:        return "YES";
    case OPTION_NO:         return "NO";

    case OPTION_ATTACK:     return "ATTACK";
    case OPTION_PUSH:       return "PUSH";
    case OPTION_GRAB:       return "GRAB";
    case OPTION_RELEASE:    return "RELEASE";
    case OPTION_LOOT:       return "LOOT";
    case OPTION_FIRE:       return "FIRE";
    case OPTION_MOVECURSOR: return "MOVE CURSOR";

    case OPTION_TEST:       return "TEST";
    case OPTION_LOAD_MAP:   return "LOAD MAP";
    case OPTION_TEST_RNG:   return "TEST RNG";
    case OPTION_REVIVE:     return "REVIVE";
    default:
        ETODO(OPTION);
        return NULL;
    }
}

int game_init(Pixel* draw_canvas_pixels, int draw_canvas_w, int draw_canvas_h){

    game.active = 1;

    game.draw_canvas = (Surface){draw_canvas_pixels, draw_canvas_w, draw_canvas_h, draw_canvas_w};

    if(draw_canvas_w * draw_canvas_h < game.draw_canvas.w * game.draw_canvas.h || draw_canvas_w < game.draw_canvas.w){
        ERROR("draw_canvas_pixels does not have minimum required capacity\n");
        return 1;
    }

    game.camera.x = 0;
    game.camera.y = 0;

    if(game.camera.w <= 0) game.camera.w = 24 * TILEW;
    if(game.camera.h <= 0) game.camera.h = 18 * TILEH;

    game.mousex = 0;
    game.mousey = 0;

    load_map(0);

    game.update = level_update;

    if(game.active != 1){
        game.active = 0;
        return 1;
    }
    
    return 0;
}

#endif // =====================  END OF FILE GAME_C ===========================