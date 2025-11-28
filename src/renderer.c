#ifndef DRAW_C
#define DRAW_C

#include "renderer.h"
#include "game.h"
#include "assets.h"
#include "maps.h"
#include "util.h"


static const char console_palette[] = {' ', 'X', '#', '\'', '\'', '\'', '\'', '\'', '\'', '\'', '\'', '\'', '\'', '\'', '\'', '\'', 'p', '@', '\'', '\'', '\'', '\'', '\'', '\'', '\'', '\'', '\'', '\'', '\'', '\'', '\'', '\''};
static const char console_general_palette[] = {'*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*'};
static const char console_map_palette[]     = {' ', 'X', '#'};
static const char console_player_sym        = 'p';
static const char console_enemy1_sym        = '@';


static const uint32_t palette[] = {
    0x00000000, 0xff212121, 0xff616161, 0xffdcdcdc,
    0xff080845, 0xff0d0d7a, 0xff1313b0, 0xff1919e6,
    0xff084508, 0xff0d7a0d, 0xff13b013, 0xff19e619,
    0xff450808, 0xff7a0d0d, 0xffb01313, 0xffe61919,
    0xff084545, 0xff0d7a7a, 0xff13b0b0, 0xff19e6e6,
    0xff082644, 0xff0d497a, 0xff136db0, 0xff1991e6,
};
static const uint32_t enitity_color[]   = {
    [ENTITY_NONE]   = 0x00000000,
    [ENTITY_PLAYER] = 0xffdcdcdc,
    [ENTITY_ENEMY1] = 0xff1919e6
};

static inline Surface create_surface(Pixel* pixels, int w, int h, int stride){
    return (Surface){.pixels = pixels, .w = w, .h = h, .stride = stride};
}


static inline Surface create_subcanvas(const Surface surface, int x, int y, int w, int h){
    return (Surface){
        .pixels = surface.pixels + x + y * surface.stride,
        .w      = (surface.w - x < w)? surface.w - x : w,
        .h      = (surface.h - y < h)? surface.h - y : h,
        .stride = surface.stride - x
    };
}

static inline uint32_t blend_color_channel(const uint32_t ct, const uint32_t at, const uint32_t cb){
    return (ct * at + (cb * (255 - at))) / 255;
}

static inline uint32_t blend_colors(const uint32_t ct, const uint32_t cb){
    const uint32_t rb = (cb >>  0) & 0xFF;
    const uint32_t gb = (cb >>  8) & 0xFF;
    const uint32_t bb = (cb >> 16) & 0xFF;
    const uint32_t ab = (cb >> 24) & 0xFF;
    const uint32_t rt = (ct >>  0) & 0xFF;
    const uint32_t gt = (ct >>  8) & 0xFF;
    const uint32_t bt = (ct >> 16) & 0xFF;
    const uint32_t at = (ct >> 24) & 0xFF;
    const uint32_t ro = blend_color_channel(rt, at, rb);
    const uint32_t go = blend_color_channel(gt, at, gb);
    const uint32_t bo = blend_color_channel(bt, at, bb);
    return (ro << 0) | (go << 8) | (bo << 16) | (ab << 24);
}


void clear_rect(Surface surface, int _x, int _y, int w, int h, Color color){
    const int xrange = (surface.w < _x + w)? surface.w - 1 : _x + w;
    const int yrange = (surface.h < _y + h)? surface.h - 1 : _y + h;

    for(int x = (_x < 0)? 0 : _x; x < xrange; x+=1){
        for(int y = (_y < 0)? 0 : _y; y < yrange; y+=1){
            surface.pixels[x + y * surface.stride] = color;
        }
    }
}

#define clearrect(surface, rect, color) clear_rect((surface), (rect).x, (rect).y, (rect).w, (rect).h, color)

void fill_rect(Surface surface, int _x, int _y, int w, int h, Color color){

    const int xrange = (surface.w < _x + w)? surface.w - 1 : _x + w;
    const int yrange = (surface.h < _y + h)? surface.h - 1 : _y + h;

    for(int x = (_x < 0)? 0 : _x; x < xrange; x+=1){
        for(int y = (_y < 0)? 0 : _y; y < yrange; y+=1){
            surface.pixels[x + y * surface.stride] = blend_colors(color, surface.pixels[x + y * surface.stride]);
        }
    }
}

#define fillrect(surface, rect, color) fill_rect((surface), (rect).x, (rect).y, (rect).w, (rect).h, color)

void draw_rect(Surface surface, int _x, int _y, int w, int h, Color color){

    const int xrange = (surface.w < _x + w)? surface.w - 1 : _x + w;
    const int yrange = (surface.h < _y + h)? surface.h - 1 : _y + h;
    _x = (_x < surface.w)? _x : surface.w;
    _y = (_y < surface.h)? _y : surface.h;

    for(int x = (_x < 0)? 0 : _x; x < xrange; x+=1){
        surface.pixels[x + _y * surface.stride]     = blend_colors(color, surface.pixels[x + _y * surface.stride]);
        surface.pixels[x + yrange * surface.stride] = blend_colors(color, surface.pixels[x + yrange * surface.stride]);
    }
    for(int y = (_y < 0)? 0 : _y; y < yrange; y+=1){
        surface.pixels[_x + y * surface.stride]     = blend_colors(color, surface.pixels[_x + y * surface.stride]);
        surface.pixels[xrange + y * surface.stride] = blend_colors(color, surface.pixels[xrange + y * surface.stride]);
    }
    surface.pixels[xrange + yrange * surface.stride] = blend_colors(color, surface.pixels[xrange + yrange * surface.stride]);
}

#define drawrect(surface, rect, color) draw_rect((surface), (rect).x, (rect).y, (rect).w, (rect).h, color)

int copy_sprite(
    Surface surface,
    const unsigned char* spritesheet,
    int sprites_per_row,
    int stride,
    int spritew, int spriteh,
    int x, int y, int _sprite,
    const Color* palette
){

    if(!palette) return 1;

    const unsigned char* const sprite =
        spritesheet +
        (_sprite % sprites_per_row) * spritew +
        ((int) (_sprite / sprites_per_row)) * spriteh * stride;

    int i;
    int j0;
    int irange = spritew;
    int jrange = spriteh;

    if(x >= surface.w || y >= surface.h || x < -spritew || y < -spriteh) return 0;

    if(x < 0){
        jrange = spritew + x;
        j0 = -x;
    } else if(x + spritew >= surface.w){
        jrange = surface.w - x;
        j0 = 0;
    } else{
        jrange = spritew;
        j0 = 0;
    }

    if(y < 0){
        irange = spriteh + y;
        i = -y;
    } else if(y + spriteh >= surface.h){
        irange = surface.h - y;
        i = 0;
    } else{
        irange = spriteh;
        i = 0;
    }

    for( ; i < irange; i+=1){
        for(int j = j0; j < jrange; j+=1){
            surface.pixels[(y + i) * surface.stride + (x + j)] = blend_colors(
                palette[sprite[i * stride + j]],
                surface.pixels[(y + i) * surface.stride + (x + j)]
            );
        }
    }

    return 0;
}

void render_text(Surface surface, int x, int y, const char* txt, uint32_t color){

    const uint32_t _palette[2] = {0x00000000, color};

    if(!txt) txt = "(null)";

    for(int i = 0; txt[i]; i+=1){

        if(txt[i] == '\n'){
            y +=FONT_SIZE;
            continue;
        }
        int _sprite = get_sprite_from_char(txt[i]);
        if(_sprite < 0) _sprite = FONT_SPACE;

        const unsigned char* const sprite =
        fontsheet +
        (_sprite % FONT_ELEMENTS_PER_ROW) * FONT_SIZE +
        ((int) (_sprite / FONT_ELEMENTS_PER_ROW)) * FONT_SIZE * FONT_STRIDE;

    int i;
    int j0;
    int irange = FONT_SIZE;
    int jrange = FONT_SIZE;

    if(x >= surface.w || y >= surface.h || x < -FONT_SIZE || y < -FONT_SIZE) continue;

    if(x < 0){
        jrange = FONT_SIZE + x;
        j0 = -x;
    } else if(x + FONT_SIZE >= surface.w){
        jrange = surface.w - x;
        j0 = 0;
    } else{
        jrange = FONT_SIZE;
        j0 = 0;
    }

    if(y < 0){
        irange = FONT_SIZE + y;
        i = -y;
    } else if(y + FONT_SIZE >= surface.h){
        irange = surface.h - y;
        i = 0;
    } else{
        irange = FONT_SIZE;
        i = 0;
    }

    for( ; i < irange; i+=1){
        for(int j = j0; j < jrange; j+=1){
            surface.pixels[(y + i) * surface.stride + (x + j)] = blend_colors(
                sprite[i * FONT_STRIDE + j]? color : (uint32_t) 0x00000000,
                surface.pixels[(y + i) * surface.stride + (x + j)]
            );
        }
    }

        x+=FONT_SIZE;
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

static void console_draw_map(){

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
        game.tmp_str[game.tmp_str_size] = '\0';
        render_text(game.draw_canvas, (game.camera.w - game.tmp_str_size * FONT_SIZE) / 2, 0, game.tmp_str, 0xFF991122);
        for(int i = 0; i < game.option_count; i+=1){
            const char* const option_str = get_option_str(game.options[i]);
            const int option_str_len = _str_len(option_str);
            render_text(
                game.draw_canvas,
                (game.camera.w - option_str_len * FONT_SIZE) / 2,
                FONT_SIZE + (i * (game.camera.h - FONT_SIZE)) / game.option_count,
                option_str, (i == game.selected_option)? 0xFF119922 : 0xFF111111
            );
        }
    }
    else if(game.draw_mode == DRAW_MODE_CONSOLE){
        printf("\x1B[2J\x1B[H\n");
        console_draw_map();
        printf("%.*s\n", game.tmp_str_size, game.tmp_str);
        printf("options:\n");
        for(int i = 0; i < game.option_count; i+=1){
            printf("%c%i- %s\n", (i == game.selected_option)? '*' : ' ', i, get_option_str(game.options[i]));
        }
    }
    else{
        VERROR("Invalid draw_mode %i", game.draw_mode);
    }

}

#endif // =====================  END OF FILE DRAW_C ===========================