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

// pass max_distance = -1.0f to check whether target is visible at all from origin point
int is_target_visible_from(const Map map, int targetx, int targety, int fromx, int fromy, float max_distance){

    const Ray ray = prepare_ray(
        fromx,
        fromy,
        targetx,
        targety
    );
    Ray rayh = ray;
    Ray rayv = ray;

    if(max_distance < 0.0f) max_distance = (float) sqrt((float) distance2(targetx, targety, fromx, fromy));

    for(int hcollision = 0; !hcollision && rayh.r < max_distance; hcollision = ray_steph(map, &rayh));

    for(int vcollision = 0; !vcollision && rayv.r < max_distance; vcollision = ray_stepv(map, &rayv));

    const float r = MIN(rayh.r, rayv.r);

    return !(r < max_distance);
}

void move_towards_visible(Entity* self, int targetx, int targety){

    if(ABS(self->x - targetx) > ABS(self->y - targety)){ // test dx movement first
        if(self->x != targetx){
            const int dx = (self->x < targetx)? TILEW : -TILEW;
            if(!get_tile(game.map, (self->x + dx) / TILEW, self->y / TILEH)){
                if(is_target_visible_from(game.map, targetx, targety, self->x + dx, self->y, -1.0f)){
                    self->x += dx;
                    return;
                }
            }
        }
        if(self->y != targety){
            const int dy = (self->y < targety)? TILEH : -TILEH;
            if(!get_tile(game.map, self->x / TILEW, (self->y + dy) / TILEH)){
                if(is_target_visible_from(game.map, targetx, targety, self->x, self->y + dy, -1.0f)){
                    self->y += dy;
                }
            } 
        }
    }
    else{ // test dy movement first
        if(self->y != targety){
            const int dy = (self->y < targety)? TILEH : -TILEH;
            if(!get_tile(game.map, self->x / TILEW, (self->y + dy) / TILEH)){
                if(is_target_visible_from(game.map, targetx, targety, self->x, self->y + dy, -1.0f)){
                    self->y += dy;
                    return;
                }
            } 
        }
        if(self->x != targetx){
            const int dx = (self->x < targetx)? TILEW : -TILEW;
            if(!get_tile(game.map, (self->x + dx) / TILEW, self->y / TILEH)){
                if(is_target_visible_from(game.map, targetx, targety, self->x + dx, self->y, -1.0f)){
                    self->x += dx;
                }
            }
        }
    }
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
        }
        else if(is_target_visible_from(game.map, game.player.x, game.player.y, self->x, self->y, -1.0f)){
            move_towards_visible(self, game.player.x, game.player.y);
            self->targetx = game.player.x;
            self->targety = game.player.y;
        }
        else{
            if(self->x == self->targetx && self->y == self->targety){
                self->state &= ~STATE_ALERTED;
            }
            else{
                move_towards_visible(self, self->targetx, self->targety);
            }
        }
        copySprite(
            game.draw_canvas,
            self->x - game.camera.x,
            self->y - game.camera.y,
            (self->state & STATE_ALERTED)? SPRITE_ALERT : self->orientation + SPRITE_ORIENTATION,
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

    const int can_see_player = is_target_visible_from(game.map, game.player.x, game.player.y, self->x, self->y, -1.0f);

    copySprite(
        game.draw_canvas,
        self->x - game.camera.x,
        self->y - game.camera.y,
        can_see_player? SPRITE_ALERT : self->orientation + SPRITE_ORIENTATION,
        enemy1_palette
    );

    if(can_see_player){
        self->targetx = game.player.x;
        self->targety = game.player.y;
        self->state |= STATE_ALERTED;
        if((ABS(self->x - game.player.x) <= TILEW) && (ABS(self->y - game.player.y) <= TILEH)){
            game.player.state = STATE_DEAD;
        }
    } else{
        if(--self->chill < 0){
            self->chill = ENEMY1_CHILL;
            self->orientation = (self->orientation + 1) % ORIENT_COUNT;
        }
    }

    return 0;
}

#endif // =====================  END OF FILE ENTITIES_HEADER ===========================