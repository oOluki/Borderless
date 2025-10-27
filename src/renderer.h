#ifndef RENDERER_HEADER
#define RENDERER_HEADER

#include "sprites.h"
#include "begin.h"


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

int copy_sprite(Surface surface, int x, int y, int _sprite, const Color* palette){

    if(!palette) return 1;

    const unsigned char* const sprite =
        sprite_sheet +
        (_sprite % SPRITES_PERROW) * SPRITE_DIM +
        ((int) (_sprite / SPRITES_PERROW)) * SPRITE_DIM * SPRITE_SHEET_STRIDE;

    int i;
    int j0;
    int irange = SPRITE_DIM;
    int jrange = SPRITE_DIM;

    if(x >= surface.w || y >= surface.h || x < -SPRITE_DIM || y < -SPRITE_DIM) return 0;

    if(x < 0){
        jrange = SPRITE_DIM + x;
        j0 = -x;
    } else if(x + SPRITE_DIM >= surface.w){
        jrange = surface.w - x;
        j0 = 0;
    } else{
        jrange = SPRITE_DIM;
        j0 = 0;
    }

    if(y < 0){
        irange = SPRITE_DIM + y;
        i = -y;
    } else if(y + SPRITE_DIM >= surface.h){
        irange = surface.h - y;
        i = 0;
    } else{
        irange = SPRITE_DIM;
        i = 0;
    }

    for( ; i < irange; i+=1){
        for(int j = j0; j < jrange; j+=1){
            surface.pixels[(y + i) * surface.stride + (x + j)] = blend_colors(
                palette[sprite[i * SPRITE_SHEET_STRIDE + j]],
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
            y +=SPRITE_DIM;
            continue;
        }
        const int sprite = get_sprite_from_char(txt[i]);

        copy_sprite(surface, x, y, (sprite < 0)? SPRITE_SPACE : sprite, _palette);

        x+=SPRITE_DIM;
    }

}


static const char console_palette[] = {' ', 'X', '#', '\'', '\'', '\'', '\'', '\'', '\'', '\'', '\'', '\'', '\'', '\'', '\'', '\'', 'p', '@', '\'', '\'', '\'', '\'', '\'', '\'', '\'', '\'', '\'', '\'', '\'', '\'', '\'', '\''};

static const char console_general_palette[] = {'*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*'};
static const char console_map_palette[]     = {' ', 'X', '#'};
static const char console_player_palette    = 'p';
static const char console_enemy1_palette    = '@';


static const uint32_t general_palette[] = {0x00000000, 0xff050305, 0xff41102e, 0xff35174a, 0xff3a1f6f, 0xff430e27, 0xff10106a, 0xff2e2e8a, 0xff524c67, 0xff0f6c40, 0xff3f5188, 0xff287377, 0xff8a2e36, 0xff955e50, 0xff683e98, 0xff7504d2, 0xff735221, 0xff89931a, 0xff484861, 0xff7c698c, 0xff629627, 0xff828282, 0xff0875e2, 0xff537194, 0xff2cb41d, 0xff499263, 0xff1b363b, 0xff077283, 0xff12edbe, 0xff56e6eb, 0xffa1fcfc, 0xffd6ffff};
static const uint32_t map_palette[]     = {0x00000000, 0xfff5d6e8, 0xff3c2845, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0xffcc1e24, 0xff0909d7, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000};
static const uint32_t player_palette[]  = {0x00000000, general_palette[12], general_palette[13]};
static const uint32_t enemy1_palette[]  = {0x00000000, general_palette[06], general_palette[07]};


#define copySprite(surface, x, y, _sprite, palette) copy_sprite(surface, x, y, _sprite, palette)


#endif // =====================  END OF FILE RENDERER_HEADER ===========================