#ifndef GAME_C
#define GAME_C

#include "game.h"
#include "maps.h"
#include "entities.h"
#include "bfs_and_raycast.h"
#include "renderer.h"

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

int _load_buttons(const int buttons[BUTTON_COUNT]){
    game.button_count = 0;
    for(int i = 0; i < BUTTON_COUNT && buttons[i] != BUTTON_NONE; i+=1){
        game.buttons[game.button_count++] = buttons[i];
    }
    return 0;
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

#define loadMap(map) load_map(map, map##w, map##h)

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

static int press_button(int buttonid){
    switch (buttonid)
    {
    case BUTTON_NONE:
        return 0;
    case BUTTON_QUIT:
        game.active = 0;
        return 1;
    case BUTTON_PLAY:
        game.entity_count = 0;
        game.button_count = 0;
        game.task_count   = 0;
        loadMap(map1);
        game.update = level_update;
        level_update(CMD_DISPLAY);
        return 1;
    default:
        VERROR("Invalid buttonid %i\n", buttonid);
        return 0;
    }
}

static const char* get_button_name(int buttonid){
    switch (buttonid)
    {
    default:
    case BUTTON_NONE: return "BUTTON_NONE";
    case BUTTON_QUIT: return "QUIT";
    case BUTTON_PLAY: return "PLAY";
    }
}

static int console_draw_tile(int tile){
    const int tile_data = TILE_DATA(tile);
    switch (TILE_TYPE(tile))
    {
    case TILETYPE_NONE:
        printf("  ");
        return 0;
    case TILETYPE_TILE:
        printf("%c%c", console_map_palette[tile_data], console_map_palette[tile_data]);
        return 0;
    case TILETYPE_PLAYER:
        if(game.player.state & STATE_ALIVE){
            printf("%c%c", console_player_sym, simple_console_font[FONT_ORIENTATION + game.player.orientation]);
            return 0;
        }
        printf("%c%c", console_player_sym, simple_console_font[FONT_DEAD]);
        return 0;
    case TILETYPE_ENTITY:
        if(game.entities[tile_data].state & STATE_ALIVE){
            if(game.entities[tile_data].state & STATE_ALERTED)
                printf("%c%c", console_enemy1_sym, simple_console_font[FONT_ALERT]);
            else
                printf("%c%c", console_enemy1_sym, simple_console_font[FONT_ORIENTATION + game.entities[tile_data].orientation]);
            return 0;
        }
        printf("%c%c", console_enemy1_sym, simple_console_font[FONT_DEAD]);
        return 0;
    default:
        VERROR("Invalid tile type %i", (int) TILE_TYPE(tile));
        return 1;
    }
}

void console_draw_map(){

    if(!game.map.map) return;

    const int ioffset = game.camera.y / TILEH;
    const int joffset = game.camera.x / TILEW;
    const int irange  = 1 + game.camera.h / TILEH;
    const int jrange  = 1 + game.camera.w / TILEW;

    printf("    ");
    for(int j = 0; j < jrange; j+=2){
        printf("%2i  ", j);
    }

    printf("\n");

    for(int i = 0; i < irange; i+=1){
        printf("%2i- ", i);
        for(int j = 0; j < jrange; j+=1){
            const Tile tile = get_tile(game.map, j + joffset, i + ioffset);
            if(console_draw_tile(tile)){
                VERROR("Could not draw tile %16llx", (long long) tile);
            }
        }
        printf("\n");
    }

    for(int j = 1; j < jrange; j+=2){
        printf("  %2i", j);
    }
    printf("\n");
}


static int graphics_draw_tile(int tile, int x, int y){
    const int tile_data = TILE_DATA(tile);
    switch (TILE_TYPE(tile))
    {
    case TILETYPE_NONE:
        return 0;
    case TILETYPE_TILE:
        fill_rect(game.draw_canvas, x, y, TILEW, TILEH, palette[tile_data]);
        return 0;
    case TILETYPE_PLAYER:{
        if(TILEW > entity_sprite_size) x += (TILEW - entity_sprite_size) / 2;
        if(TILEH > entity_sprite_size) y += (TILEH - entity_sprite_size) / 2;
        return copy_sprite(
            game.draw_canvas, entity_spritesheet,
            1, entity_sprite_size,
            entity_sprite_size, entity_sprite_size,
            x, y, (game.player.state & STATE_ALIVE)? game.player.orientation : ENTITY_SPRITE_DEAD,
            (Color[3]){0x00000000, palette[1], enitity_color[ENTITY_PLAYER]}
        );
    }
    case TILETYPE_ENTITY:{
        if(TILEW > entity_sprite_size) x += (TILEW - entity_sprite_size) / 2;
        if(TILEH > entity_sprite_size) y += (TILEH - entity_sprite_size) / 2;
        if(game.entities[tile_data].state & STATE_ALERTED){
            copy_sprite(
                game.draw_canvas,
                fontsheet, FONT_ELEMENTS_PER_ROW,
                FONT_STRIDE, FONT_SIZE, FONT_SIZE,
                x, y - entity_sprite_size, FONT_ALERT,
                (Color[3]){0x00000000, enitity_color[ENTITY_ENEMY1]}
            );
        }
        return copy_sprite(
            game.draw_canvas, entity_spritesheet,
            1, entity_sprite_size,
            entity_sprite_size, entity_sprite_size,
            x, y, (game.entities[tile_data].state & STATE_ALIVE)? game.entities[tile_data].orientation : ENTITY_SPRITE_DEAD,
            (Color[3]){0x00000000, palette[1], enitity_color[ENTITY_ENEMY1]}
        );
    }
    default:
        VERROR("Invalid tile type %i", (int) TILE_TYPE(tile));
        return 1;
    }
}

static void graphics_draw_map(){
    if(!game.map.map) return;

    const int ioffset = game.camera.y / TILEH;
    const int joffset = game.camera.x / TILEW;
    const int irange  = 1 + game.camera.h / TILEH;
    const int jrange  = 1 + game.camera.w / TILEW;

    for(int i = 0; i < irange; i+=1){
        for(int j = 0; j < jrange; j+=1){
            const Tile tile = get_tile(game.map, j + joffset, i + ioffset);
            if(graphics_draw_tile(tile, j * TILEW - (game.camera.x % TILEW), i * TILEH - (game.camera.y % TILEH))){
                VERROR("Could not draw tile %16llx", (long long) tile);
            }
        }
    }
}

void draw(){

    if(game.draw_mode == DRAW_MODE_NONE){
        return ;
    }
    else if(game.draw_mode == DRAW_MODE_GRAPHIC){
        clear_rect(game.draw_canvas, 0, 0, game.camera.w, game.camera.h, BACKGROUND_COLOR);
        graphics_draw_map();
        for(int i = 0; i < game.button_count; i+=1){
            const char* const button_name = get_button_name(game.buttons[i]);
            const int button_name_len = _str_len(button_name);
            render_text(
                game.draw_canvas,
                (game.camera.w - button_name_len * FONT_SIZE) / 2,
                (i * game.camera.h) / game.button_count,
                button_name, (i == game.selected_button)? 0xFF22AA11 : 0xFF111111
            );
        }
    }
    else if(game.draw_mode == DRAW_MODE_CONSOLE){
        printf("\x1B[2J\x1B[H\n");
        console_draw_map();
        printf("options:\n");
        for(int i = 0; i < game.button_count; i+=1){
            printf("%c%i- %s\n", (i == game.selected_button)? '*' : ' ', i, get_button_name(game.buttons[i]));
        }
    }
    else{
        VERROR("Invalid draw_mode %i", game.draw_mode);
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
        game.update = button_select_update;
        loadButtons(BUTTON_QUIT, BUTTON_PLAY);
        button_select_update(CMD_DISPLAY);
        return 0;
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
        break;
    case CMD_LEFT:
        game.player.orientation = (game.player.orientation + ORIENT_COUNT - 1) % ORIENT_COUNT;
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

int button_select_update(int cmd){

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
        if(game.button_count) game.selected_button = (game.selected_button + 1) % game.button_count;
        break;
    case CMD_UP:
        if(game.button_count) game.selected_button = (game.selected_button == 0)? game.button_count - 1 : game.selected_button - 1;
        break;
    case CMD_MOUSECLICK:
    case CMD_ENTER:
        if(game.selected_button >= 0 && game.selected_button < game.button_count){
            return press_button(game.buttons[game.selected_button]);
        }
        return 0;
    
    default:
        return 0;
    }

    draw();

    return 0;
}

int game_init(Pixel* draw_canvas_pixels, int draw_canvas_w, int draw_canvas_h){

    game.draw_canvas = create_surface(draw_canvas_pixels, draw_canvas_w, draw_canvas_h, draw_canvas_w);

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