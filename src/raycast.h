#ifndef RAYCAST_HEADER
#define RAYCAST_HEADER

#include "game.h"


Ray prepare_ray(int x, int y, int targetx, int targety){
    return (Ray){
        .x = (float) x,
        .y = (float) y,
        .r = 0,
        .cos = (float)(targetx - x) / sqrtf((targetx - x) * (targetx - x) + (targety - y) * ( targety - y)),
        .sin = (float)(targety - y) / sqrtf((targetx - x) * (targetx - x) + (targety - y) * ( targety - y))
    };
}

int ray_steph(const Map map, Ray* ray){

    if(ray->cos == 0){
        if(ray->sin == 0) return -1;
        ray->y  = INFINITY;
        ray->r = INFINITY;
        return 0;
    }

    const float dx = (ray->cos > 0)? 1.0f * TILEW : -1.0f * TILEW;
    const float dy = (dx * ray->sin) / ray->cos;

    ray->x += dx;
    ray->y += dy;
    ray->r += sqrtf(dx * dx + dy * dy);

    return get_tile(map, (int) (ray->x / TILEW) , (int) (ray->y / TILEH));
}

int ray_stepv(const Map map, Ray* ray){

    if(ray->sin == 0){
        if(ray->cos == 0) return -1;
        ray->x  = INFINITY;
        ray->r = INFINITY;
        return 0;
    }

    const float dy = (ray->sin > 0)? 1.0f * TILEH : -1.0f * TILEH;
    const float dx = (dy * ray->cos) / ray->sin;

    ray->x += dx;
    ray->y += dy;
    ray->r += sqrtf(dx * dx + dy * dy);

    return get_tile(map, (int) (ray->x / TILEW) , (int) (ray->y / TILEH));
}


#endif // =====================  END OF FILE RAYCAST_HEADER ===========================