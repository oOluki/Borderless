#ifndef RENDERER_HEADER
#define RENDERER_HEADER

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


void fill(Surface surface, uint32_t color){
    for(int i = 0; i < surface.h; i+=1){
        for(int j = 0; j < surface.w; j+=1){
            surface.pixels[i * surface.stride + j] = color;
        }
    }
}

void fill_rect(Surface surface, int x, int _y, int w, int h, uint32_t color){

    const int xrange = (surface.w < x + w)? surface.w : x + w;
    const int yrange = (surface.h < _y + h)? surface.h : _y + h;

    for(x = (x < 0)? 0 : x; x < xrange; x+=1){
        for(int y = (_y < 0)? 0 : _y; y < yrange; y+=1){
            surface.pixels[x + y * surface.stride] = color;
        }
    }
}

#define fillrect(surface, rect, color) fill_rect((surface), (rect).x, (rect).y, (rect).w, (rect).h, color)

void draw_rect(Surface surface, int _x, int _y, int w, int h, uint32_t color){

    const int xrange = (surface.w < _x + w)? surface.w : _x + w;
    const int yrange = (surface.h < _y + h)? surface.h : _y + h;
    _x = (_x < surface.w)? _x : surface.w;
    _y = (_y < surface.h)? _y : surface.h;

    for(int x = (_x < 0)? 0 : _x; x < xrange; x+=1){
        surface.pixels[x + _y * surface.stride] = color;
        surface.pixels[x + yrange * surface.stride] = color;
    }
    for(int y = (_x < 0)? 0 : _x; y < yrange; y+=1){
        surface.pixels[_x + y * surface.stride] = color;
        surface.pixels[xrange + y * surface.stride] = color;
    }
}

#define drawrect(surface, rect, color) draw_rect((surface), (rect).x, (rect).y, (rect).w, (rect).h, color)

void fill_circle(Surface surface, int _x, int _y, int r, uint32_t color){

    const int xrange = MIN(_x + r, surface.w);
    const int yrange = MIN(_y + r, surface.h);

    for(int x = MAX(_x - r, 0); x < xrange; x+=1){
        for(int y = MAX(_y - r, 0); y < yrange; y+=1){
            if((x - _x) * (x - _x) + (y - _y) * (y - _y) <= r * r){
                surface.pixels[y * surface.stride + x] = color;
            }
        }
    }

}

void draw_circle(Surface surface, int _x, int _y, int r, uint32_t color){

    for(int x = -r; x < r; x+=1){
        int y = (int) (Q_rsqrt((float)(r * r - x * x)));
        if(in_sbounds(surface, x + _x, y + _y)) surface.pixels[(y + _y) * surface.stride + (x + _x)];
        if(in_sbounds(surface, x + _x, -y + _y)) surface.pixels[(-y + _y) * surface.stride + (x + _x)];
    }

}



#endif // =====================  END OF FILE RENDERER_HEADER ===========================