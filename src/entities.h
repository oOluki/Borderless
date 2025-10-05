#ifndef ENTITIES_HEADER
#define ENTITIES_HEADER

#include "begin.h"
#include "raycast.h"

int update_enemy1(Game* game, Entity* self){
    if(!collide_rect(game->camera, (Rect){self->x, self->y, TILEW, TILEH})){
        return 0;
    }

    const Ray ray = prepare_ray(
        self->x,
        self->y,
        game->player.x,
        game->player.y
    );
    Ray rayh = ray;
    Ray rayv = ray;

    const float d2 = ((float) distance2(self->x, self->y, game->player.x, game->player.y));

    int hcollision = 0;
    
    for(; !hcollision && rayh.r2 < d2; hcollision = ray_steph(game->map, &rayh)){
        //fill_rect(
        //    game->draw_canvas,
        //    (int) ((rayh.x * TILEW >> 8) - game->camera.x),
        //    (int) ((rayh.y * TILEH >> 8) - game->camera.y),
        //    TILEW / 4, TILEH / 4, 0x000000CC
        //);
    }

    int vcollision = 0;

    for(; !vcollision && rayv.r2  < d2; vcollision = ray_stepv(game->map, &rayv)){
        //fill_rect(
        //    game->draw_canvas,
        //    (int) ((rayv.x * TILEW >> 8) - game->camera.x),
        //    (int) ((rayv.y * TILEH >> 8) - game->camera.y),
        //    TILEW / 4, TILEH / 4, 0x001111AA
        //);
    }

    const float r2 = MIN(rayh.r2, rayv.r2);

    if(game->debug){
        printf(
            "r2=%lf\n"
            "d2=%lf\n",
            sqrt(r2),
            sqrt(d2)
        );
    }

    rayh = ray;
    rayv = ray;

    for(int i = 0; !ray_steph(game->map, &rayh) && rayh.r2 < r2 && i < 100; i+=1){
        fill_rect(
            game->draw_canvas,
            (int) (rayh.x) - game->camera.x,
            (int) (rayh.y) - game->camera.y,
            TILEW / 4, TILEH / 4, 0xFF0000CC
        );
    }

    for(int i = 0; !ray_stepv(game->map, &rayv) && rayv.r2 < r2 && i < 100; i+=1){
        fill_rect(
            game->draw_canvas,
            (int) (rayv.x) - game->camera.x,
            (int) (rayv.y) - game->camera.y,
            TILEW / 4, TILEH / 4, 0xFFCC0000
        );
    }

    fill_rect(
        game->draw_canvas,
        self->x - game->camera.x, self->y - game->camera.y,
        TILEW, TILEH,
        (hcollision && vcollision)? 0xFF1111AA : palettes[PALETTE_MAP][TILE_ENEMY1]
    );
    return 0;
} // 1048576 589824

#endif // =====================  END OF FILE ENTITIES_HEADER ===========================