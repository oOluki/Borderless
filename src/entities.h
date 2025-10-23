#ifndef ENTITIES_HEADER
#define ENTITIES_HEADER

#include "game.h"
#include "raycast.h"
#include "renderer.h"

static inline int facing_entity(int x, int y, int orientation, int targetx, int targety){
    switch (orientation)
    {
    case ORIENT_UP:
        return (targety - y) < 0;
    case ORIENT_RIGHT:
        return (targetx - x) > 0;
    case ORIENT_DOWN:
        return (targety - y) > 0;
    case ORIENT_LEFT:
        return (targetx - x) < 0;
    default:
        return (targetx == x && targety == y);
    }
}

static inline int orientation_direction(int orientation, int* dx, int* dy){
    switch (orientation)
    {
    case ORIENT_UP:
        if(dx) *dx = 0;
        if(dy) *dy = -1;
        return 0;
    case ORIENT_RIGHT:
        if(dx) *dx = 1;
        if(dy) *dy = 0;
        return 0;
    case ORIENT_DOWN:
        if(dx) *dx = 0;
        if(dy) *dy = 1;
        return 0;
    case ORIENT_LEFT:
        if(dx) *dx = -1;
        if(dy) *dy = 0;
        return 0;
    
    default:
        return 1;
    }
}

int is_target_visible_from(const Map map, int targetx, int targety, int fromx, int fromy){

    const Ray ray = prepare_ray(
        fromx,
        fromy,
        targetx,
        targety
    );
    Ray rayh = ray;
    Ray rayv = ray;

    const float d = sqrtf((float) distance2(fromx, fromy, targetx, targety));

    for(int hcollision = 0; !hcollision && rayh.r < d; hcollision = ray_steph(map, &rayh));

    for(int vcollision = 0; !vcollision && rayv.r < d; vcollision = ray_stepv(map, &rayv));

    const float r = MIN(rayh.r, rayv.r);

    return !(r < d);
}

int update_enemy1(Entity* self){
    if(!collide_rect(game.camera, (Rect){self->x, self->y, TILEW, TILEH})){
        return 0;
    }

    if(!(self->state & STATE_ALIVE)){
        copySprite(game.draw_canvas, self->x - game.camera.x, self->y - game.camera.y, SPRITE_DEAD, enemy1_palette);
        return 0;
    }
    if(self->state & STATE_ALERTED){
        if((ABS(self->x - game.player.x) <= TILEW) && (ABS(self->y - game.player.y) <= TILEH)){
            game.player.state = STATE_DEAD;
        } else if(is_target_visible_from(game.map, game.player.x, game.player.y, self->x, self->y)){
            const Node next = move_towards_player(game.map.w, game.map.h, (Node){.x = self->x / TILEW, .y = self->y / TILEH});
            int update = 1;
            for(int i = 0; i < game.entity_count; i+=1){
                const Entity e = game.entities[i];
                if(!(e.state & STATE_ALIVE) || (e.state & STATE_ALERTED)){
                    continue;
                }
                if(collide_rect((Rect){.x = next.x * TILEW, .y = next.y * TILEH, TILEW, TILEH},
                (Rect){.x = e.x * TILEW   , .y = e.y * TILEH   , TILEW, TILEH})){
                    update = 0;
                    break;
                }
            }
            if(update){
                self->x = next.x * TILEW;
                self->y = next.y * TILEH;
            }
        } else{
            self->state &= ~STATE_ALERTED;
        }
        for(int i = 0; i < game.entity_count; i+=1){
            Entity* const e = game.entities + i;
            if(!(e->state & STATE_ALIVE) || (e->state & STATE_ALERTED)){
                continue;
            }
            if(facing_entity(self->x, self->y, self->orientation, e->x, e->y)){
                if(is_target_visible_from(game.map, e->x, e->y, self->x, self->y)){
                    if(distance2(self->x, self->y, e->x, e->y) < 100 * (TILEW * TILEW + TILEH * TILEH))
                        e->state |= STATE_ALERTED;
                }
            }
        }
        copySprite(
            game.draw_canvas,
            self->x - game.camera.x,
            self->y - game.camera.y,
            SPRITE_ALERT,
            enemy1_palette
        );
        return 0;
    }

    const int facing_player = facing_entity(self->x, self->y, self->orientation, game.player.x, game.player.y);

    if(!facing_player){
        if((ABS(self->x - game.player.x) <= TILEW) && (ABS(self->y - game.player.y) <= TILEH)){
            self->state = STATE_DEAD;
        }
        if(!self->chill--){
            self->chill = ENEMY1_CHILL;
            self->orientation = (self->orientation + 1) % ORIENT_COUNT;
        }
        copySprite(
            game.draw_canvas,
            self->x - game.camera.x,
            self->y - game.camera.y,
            self->orientation + SPRITE_ORIENTATION,
            enemy1_palette
        );
        return 0;
    }

    const int can_see_player = is_target_visible_from(game.map, game.player.x, game.player.y, self->x, self->y);

    copySprite(
        game.draw_canvas,
        self->x - game.camera.x,
        self->y - game.camera.y,
        can_see_player? SPRITE_ALERT : self->orientation + SPRITE_ORIENTATION,
        enemy1_palette
    );

    if(!can_see_player){
        if(--self->chill < 0){
            self->chill = ENEMY1_CHILL;
            self->orientation = (self->orientation + 1) % ORIENT_COUNT;
        }
    } else{
        self->targetx = game.player.x;
        self->targety = game.player.y;
        self->state |= STATE_ALERTED;
        if((ABS(self->x - game.player.x) <= TILEW) && (ABS(self->y - game.player.y) <= TILEH)){
            game.player.state = STATE_DEAD;
        }
    }

    return 0;
}

#endif // =====================  END OF FILE ENTITIES_HEADER ===========================