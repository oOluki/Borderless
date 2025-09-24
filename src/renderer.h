#ifndef RENDERER_HEADER
#define RENDERER_HEADER

#include <stdint.h>

typedef uint32_t Pixel;

typedef struct Surface
{
    Pixel* pixels;
    int    w;
    int    h;
    int    stride;
} Surface;


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

    for(; x < xrange; x+=1){
        for(int y = _y; y < yrange; y+=1){
            surface.pixels[x + y * surface.stride] = color;
        }
    }
}

void draw_rect(Surface surface, int _x, int _y, int w, int h, uint32_t color){

    const int xrange = (surface.w < _x + w)? surface.w : _x + w;
    const int yrange = (surface.h < _y + h)? surface.h : _y + h;
    _x = (_x < surface.w)? _x : surface.w;
    _y = (_y < surface.h)? _y : surface.h;

    for(int x = _x; x < xrange; x+=1){
        surface.pixels[x + _y * surface.stride] = color;
        surface.pixels[x + yrange * surface.stride] = color;
    }
    for(int y = _y; y < yrange; y+=1){
        surface.pixels[_x + y * surface.stride] = color;
        surface.pixels[xrange + y * surface.stride] = color;
    }
}



#endif // =====================  END OF FILE RENDERER_HEADER ===========================