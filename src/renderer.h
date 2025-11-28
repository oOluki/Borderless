#ifndef RENDERER_HEADER
#define RENDERER_HEADER

#include "begin.h"


void clear_rect(Surface surface, int _x, int _y, int w, int h, Color color);

#define clearrect(surface, rect, color) clear_rect((surface), (rect).x, (rect).y, (rect).w, (rect).h, color)

void fill_rect(Surface surface, int _x, int _y, int w, int h, Color color);

#define fillrect(surface, rect, color) fill_rect((surface), (rect).x, (rect).y, (rect).w, (rect).h, color)

void draw_rect(Surface surface, int _x, int _y, int w, int h, Color color);

#define drawrect(surface, rect, color) draw_rect((surface), (rect).x, (rect).y, (rect).w, (rect).h, color)

int copy_sprite(
    Surface surface,
    const unsigned char* spritesheet,
    int sprites_per_row,
    int stride,
    int spritew, int spriteh,
    int x, int y, int _sprite,
    const Color* palette
);

void render_text(Surface surface, int x, int y, const char* txt, uint32_t color);


#define copySprite(surface, x, y, _sprite, palette) copy_sprite(surface, x, y, _sprite, palette)


#endif // =====================  END OF FILE RENDERER_HEADER ===========================
