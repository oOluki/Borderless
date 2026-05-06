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
static const char entity_symbol[]   = {
    [ENTITY_NONE]   = '.',
    [ENTITY_PLAYER] = 'p',
    [ENTITY_ENEMY1] = '@'
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

int copy_raw(Surface surface, const unsigned char* sprite, int stride, int spritew, int spriteh, int x, int y, const uint32_t* _palette){

    if(!sprite || !palette) return 1;

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
                _palette[sprite[i * stride + j]],
                surface.pixels[(y + i) * surface.stride + (x + j)]
            );
        }
    }
    return 0;
}

int copy_sprite(Surface surface, const SpriteSheet spritesheet, int x, int y, int _sprite, const Color* _palette){

    if(!_palette || !spritesheet.spritesheet) return 1;

    DEBUG_ASSERT(spritesheet.sprites_per_row > 0);

    const unsigned char* const sprite =
        spritesheet.spritesheet +
        (_sprite % spritesheet.sprites_per_row) * spritesheet.spritew +
        ((int) (_sprite / spritesheet.sprites_per_row)) * spritesheet.spriteh * spritesheet.stride;

    return copy_raw(surface, sprite, spritesheet.stride, spritesheet.spritew, spritesheet.spriteh, x, y, _palette);
}

void render_text(Surface surface, int _x, int y, const char* txt, uint32_t color){

    const uint32_t _palette[2] = {0x00000000, color};

    if(!txt) txt = "(null)";

    int x = _x;

    for(int i = 0; txt[i]; i+=1){

        if(txt[i] == '\n'){
            y += fontsheet.spriteh;
            x = _x;
            continue;
        }
        int _sprite = get_sprite_from_char(txt[i]);
        if(_sprite < 0) _sprite = FONT_SPACE;

        copy_sprite(surface, fontsheet, x, y, _sprite, (Color[]){0x00000000, color});

        x += fontsheet.spritew;
    }

}




static int console_draw_tile(char* output, const int output_stride, Tile tile){
    const int tile_data = TILE_DATA(tile);
    const int TILETYPE = TILE_TYPE(tile);
    switch (TILETYPE)
    {
    case TILETYPE_NONE:
        for(int i = 0; i < 3; i+=1)
            for(int j = 0; j < 3; j+=1)
                output[i * output_stride + j] = ' ';
        return 0;
    case TILETYPE_TILE:
        for(int i = 0; i < 3; i+=1)
            for(int j = 0; j < 3; j+=1)
                output[i * output_stride + j] = console_map_palette[tile_data];
        return 0;
    case TILETYPE_PLAYER:
    case TILETYPE_ENTITY:{
        const Entity* entity;
        if(TILETYPE == TILETYPE_PLAYER)
            entity = &game.player;
        else
            entity = &game.entities[tile_data];

        for(int i = 0; i < 3; i+=1)
            for(int j = 0; j < 3; j+=1)
                output[i * output_stride + j] = ' ';

        if(entity->state & STATE_ALIVE){
            output[0] = (entity->state & STATE_ALERTED)? simple_console_font[FONT_ALERT] : ' ';
        }
        else{
            output[0] = simple_console_font[FONT_DEAD];
        }
        output[1 * output_stride + 1] = entity_symbol[entity->id];
        if((entity->state & STATE_ALERTED) && (entity->weapon == WEAPON_PISTOL))
            output[2] = 'p';
        switch (entity->orientation)
        {
        case ORIENT_UP:
            output[0 * output_stride + 1] = simple_console_font[FONT_UP];
            break;
        case ORIENT_RIGHT:
            output[1 * output_stride + 2] = simple_console_font[FONT_RIGHT];
            break;
        case ORIENT_DOWN:
            output[2 * output_stride + 1] = simple_console_font[FONT_DOWN];
            break;
        case ORIENT_LEFT:
            output[1 * output_stride + 0] = simple_console_font[FONT_LEFT];
            break;
        default:
            output[1 * output_stride + 0] = simple_console_font[FONT_INTERROGATION];
            break;
        }
    }
        return 0;
    default:
        for(int i = 0; i < 3; i+=1)
            for(int j = 0; j < 3; j+=1)
                output[i * output_stride + j] = '?';
        ETODO(TILETYPE);
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
        printf("%3i   ", j);
    }
    printf("\n");

    const int stride = 3 * jrange;

    for(int i = 0; i < irange; i+=1){
        char* canvas = (char*) game.draw_canvas.pixels;
        for(int j = 0; j < jrange; j+=1){
            const Tile tile = get_tile(game.map, j + joffset, i + ioffset);
            if(console_draw_tile(canvas, stride, tile)){
                ERROR("Could not draw tile %16llx", (long long) tile);
                return ;
            }
            canvas += 3;
        }
        printf("     %.*s\n", stride, ((char*) game.draw_canvas.pixels) + 0 * stride);
        printf("%3i- %.*s\n", i, stride, ((char*) game.draw_canvas.pixels) + 1 * stride);
        printf("     %.*s\n", stride, ((char*) game.draw_canvas.pixels) + 2 * stride);
    }

    printf("    ");
    for(int j = 1; j < jrange; j+=2){
        printf("   %3i", j);
    }
    printf("\n");
}

static const unsigned char* get_weapon_sprite(int weapon, int direction, int* stride, int* w, int* h){

    if(weapon < 0 || weapon > WEAPON_COUNT){
        ERROR("invalid weapon %i", weapon);
        return NULL;
    }
    if(weapon == WEAPON_NONE){
        if(w) *w = 0;
        if(h) *h = 0;
        if(stride) 0;
        return NULL;
    }

    const SpriteSheet spritesheet = weapon_spritesheets[weapon];
    if(spritesheet.sprites_per_row != 1){
        ERROR("weapon spritesheets are expected to have 1 sprite per row, weapon %i got %i instead", weapon, spritesheet.sprites_per_row);
        return NULL;
    }
    const unsigned char* sprite = spritesheet.spritesheet + spritesheet.spritew * spritesheet.spriteh * direction;

    // tests is direction is odd
    const int swap = direction % 2;
    
    if(w) *w = (swap)? spritesheet.spriteh : spritesheet.spritew;
    if(h) *h = (swap)? spritesheet.spritew : spritesheet.spriteh;
    if(stride) *stride = (swap)? spritesheet.spriteh : spritesheet.spritew;

    return sprite;
}

static int graphics_draw_tile(Tile tile, int x, int y){
    const int tile_data = TILE_DATA(tile);
    const int TILETYPE = TILE_TYPE(tile);
    switch (TILETYPE)
    {
    case TILETYPE_NONE:
        return 0;
    case TILETYPE_TILE:
        fill_rect(game.draw_canvas, x, y, TILEW, TILEH, palette[tile_data]);
        return 0;
    case TILETYPE_PLAYER:
    case TILETYPE_ENTITY:{
        const Entity* entity;
        if(TILETYPE == TILETYPE_PLAYER)
            entity = &game.player;
        else
            entity = &game.entities[tile_data];

        if(TILEW > entity_spritesheet.spritew) x += (TILEW - entity_spritesheet.spritew) / 2;
        if(TILEH > entity_spritesheet.spriteh) y += (TILEH - entity_spritesheet.spriteh) / 2;
        const int orient = entity->orientation;
        const int dx = (orient == ORIENT_RIGHT) - (orient == ORIENT_LEFT);
        const int dy = (orient == ORIENT_DOWN)  - (orient == ORIENT_UP);
        const int sw = weapon_spritesheets[entity->weapon].spritew;
        const int sh = weapon_spritesheets[entity->weapon].spriteh;
        const int w = (orient == ORIENT_UP || orient == ORIENT_DOWN)? sw : sh;
        const int h = (orient == ORIENT_UP || orient == ORIENT_DOWN)? sh : sw;
        int wstride;
        int wsw;
        int wsh;
        const unsigned char* const weapon_sprite = get_weapon_sprite(entity->weapon, entity->orientation, &wstride, &wsw, &wsh);
        copy_raw(
            game.draw_canvas, weapon_sprite, wstride, wsw, wsh,
            (dx > 0)? x + entity_spritesheet.spritew + 1 : x + dx * (w + 1),
            (dy > 0)? y + entity_spritesheet.spriteh + 1 : y + dy * (h + 1),
            (Color[3]){0x00000000, palette[1], enitity_color[entity->id]}
        );
        if(entity->state & STATE_ALERTED){
            copy_sprite(
                game.draw_canvas, fontsheet,
                x, y - entity_spritesheet.spriteh, FONT_ALERT,
                (Color[3]){0x00000000, enitity_color[entity->id]}
            );
        }
        return copy_sprite(
            game.draw_canvas, entity_spritesheet,
            x, y,
            (entity->state & STATE_ALIVE)? entity->orientation : ENTITY_SPRITE_DEAD,
            (Color[3]){0x00000000, palette[1], enitity_color[entity->id]}
        );
    }
    default:
        ETODO(TILETYPE);
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
                ERROR("Could not draw tile %llu", (long long unsigned) tile);
                return ;
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
        if(game.tmp_message_frames){
            game.tmp_str[game.tmp_str_size] = '\0';
            render_text(game.draw_canvas, (game.camera.w - game.tmp_str_size * fontsheet.spritew) / 2, 0, game.tmp_str, 0xFF991122);
            game.tmp_message_frames -= 1;
        }
        if(game.update == option_select_update) for(int i = 0; i < game.option_count; i+=1){
            const char* const option_str = get_option_str(game.options[i]);
            const int option_str_len = _str_len(option_str);
            render_text(
                game.draw_canvas,
                (game.camera.w - option_str_len * fontsheet.spritew) / 2,
                fontsheet.spriteh + (i * (game.camera.h - fontsheet.spriteh)) / game.option_count,
                option_str, (i == game.selected_option)? 0xFF119922 : 0xFF111111
            );
        }
    }
    else if(game.draw_mode == DRAW_MODE_CONSOLE){
        printf("\x1B[2J\x1B[H\n");
        console_draw_map();
        if(game.tmp_message_frames){
            printf("%.*s\n", game.tmp_str_size, game.tmp_str);
            game.tmp_message_frames -= 1;
        }
        if(game.update == option_select_update){
            printf("options:\n");
            for(int i = 0; i < game.option_count; i+=1){
                printf("%c%i- %s\n", (i == game.selected_option)? '*' : ' ', i, get_option_str(game.options[i]));
            }
        }
    }
    else{
        ERROR("Invalid draw_mode %i", game.draw_mode);
    }

}

#endif // =====================  END OF FILE DRAW_C ===========================