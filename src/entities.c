#ifndef ENTITIES_C
#define ENTITIES_C

#include "game.h"
#include "util.h"
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

int orientation_direction(int orientation, int* dx, int* dy){
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

int get_weapon_range(int weapon){
    static const int weapon_range[WEAPON_COUNT] = {
        [WEAPON_NONE]   = 0,
        [WEAPON_PISTOL] = 6
    };
    if(weapon < 0 || weapon >= WEAPON_COUNT){
        ERROR("weapon %i does not exist", weapon);
        return 0;
    }
    return weapon_range[weapon];
}

int fire_weapon(int weapon_range, int mapx, int mapy, int direction){

    int dx;
    int dy;
    if(orientation_direction(direction, &dx, &dy)){
        ERROR("invalid direction %i", direction);
        return 0;
    }

    mapx += dx;
    mapy += dy;
    
    int i = 0;
    Tile tile = get_tile(game.map, mapx, mapy);
    for(; TILE_TYPE(tile) == TILETYPE_NONE && i < weapon_range; i+=1){
        mapx += dx;
        mapy += dy;
        tile = get_tile(game.map, mapx, mapy);
    }
    if(TILE_TYPE(tile) == TILETYPE_ENTITY){
        game.entities[TILE_DATA(tile)].state &= ~STATE_CLEAN_ON_KILL;
    }
    else if(TILE_TYPE(tile) == TILETYPE_PLAYER){
        game.player.state &= ~STATE_CLEAN_ON_KILL;
    }

    return i;
}

// pass max_distance = -1.0f to check whether target is visible at all from origin point
static int is_target_visible_from(const Map map, int targetx, int targety, int fromx, int fromy, float max_distance){

    Ray rayh = prepare_ray(
        fromx,
        fromy,
        targetx,
        targety
    );
    Ray rayv = rayh;

    if(max_distance < 0.0f) max_distance = (float) sqrt((float) distance2(targetx, targety, fromx, fromy));

    for(int hcollision = 0; !hcollision && rayh.r < max_distance; hcollision = ray_steph(map, &rayh));

    for(int vcollision = 0; !vcollision && rayv.r < max_distance; vcollision = ray_stepv(map, &rayv));

    const float r = MIN(rayh.r, rayv.r);

    const Ray ray = (rayh.r < rayv.r)? rayh : rayv;

    const Tile hittile = get_tile(map, (int) (ray.x) / TILEW, (int) (ray.y) / TILEH);

    return !(r < max_distance) || TILE_TYPE(hittile) == TILETYPE_PLAYER;
}


// \returns whether it could move self or not
static int move_towards_visible(Entity* self, int targetx, int targety){

    if(ABS(self->x - targetx) > ABS(self->y - targety)){ // test dx movement first
        if(self->x != targetx){
            const int dx = (self->x < targetx)? TILEW : -TILEW;
            if(!get_tile(game.map, (self->x + dx) / TILEW, self->y / TILEH)){
                if(is_target_visible_from(game.map, targetx, targety, self->x + dx, self->y, -1.0f)){
                    move_tile(self->x / TILEW, self->y / TILEH, (self->x + dx) / TILEW, self->y / TILEH);
                    self->orientation = (dx > 0)? ORIENT_RIGHT : ORIENT_LEFT;
                    return 1;
                }
            }
        }
        if(self->y != targety){
            const int dy = (self->y < targety)? TILEH : -TILEH;
            if(!get_tile(game.map, self->x / TILEW, (self->y + dy) / TILEH)){
                if(is_target_visible_from(game.map, targetx, targety, self->x, self->y + dy, -1.0f)){
                    move_tile(self->x / TILEW, self->y / TILEH, self->x / TILEW, (self->y + dy) / TILEH);
                    self->orientation = (dy > 0)? ORIENT_DOWN : ORIENT_UP;
                    return 1;
                }
            } 
        }
    }
    else{ // test dy movement first
        if(self->y != targety){
            const int dy = (self->y < targety)? TILEH : -TILEH;
            if(!get_tile(game.map, self->x / TILEW, (self->y + dy) / TILEH)){
                if(is_target_visible_from(game.map, targetx, targety, self->x, self->y + dy, -1.0f)){
                    move_tile(self->x / TILEW, self->y / TILEH, self->x / TILEW, (self->y + dy) / TILEH);
                    self->orientation = (dy > 0)? ORIENT_DOWN : ORIENT_UP;
                    return 1;
                }
            } 
        }
        if(self->x != targetx){
            const int dx = (self->x < targetx)? TILEW : -TILEW;
            if(!get_tile(game.map, (self->x + dx) / TILEW, self->y / TILEH)){
                if(is_target_visible_from(game.map, targetx, targety, self->x + dx, self->y, -1.0f)){
                    move_tile(self->x / TILEW, self->y / TILEH, (self->x + dx) / TILEW, self->y / TILEH);
                    self->orientation = (dx > 0)? ORIENT_RIGHT : ORIENT_LEFT;
                    return 1;
                }
            }
        }
    }
    return 0;
}

int update_entity(Entity* self){

    self->state &= ~STATE_CLEAN_ON_UPDATE;

    if(!(self->state & STATE_ALIVE)){
        return 0;
    }

    if(!(self->state & STATE_ALERTED) && !collide_rect(game.camera, (Rect){self->x, self->y, TILEW, TILEH})){
        return 0;
    }

    if(self->state & STATE_ALERTED){
        if((ABS(self->x - game.player.x) <= TILEW) && (ABS(self->y - game.player.y) <= TILEH)){
            game.player.state &= ~STATE_CLEAN_ON_KILL;
        }
        else if(is_target_visible_from(game.map, game.player.x, game.player.y, self->x, self->y, -1.0f)){
            if(!move_towards_visible(self, game.player.x, game.player.y)){ // diagonal case...
                const int dx = (self->x < game.player.x)? TILEW : -TILEW;
                const int dy = (self->y < game.player.y)? TILEH : -TILEH;
                if(!get_tile(game.map, (self->x + dx) / TILEW, self->y / TILEH)){
                    move_tile(self->x / TILEW, self->y / TILEH, (self->x + dx) / TILEW, self->y / TILEH);
                    //self->x += dx;
                }
                else if(!get_tile(game.map, self->x / TILEW, (self->y + dy) / TILEH)){
                    move_tile(self->x / TILEW, self->y / TILEH, self->x / TILEW, (self->y + dy) / TILEH);
                    //self->y += dy;
                }
                else{ // this is a special corner case, where the entity is in a corner and can see through the diagonal (even though it should not)
                    self->state &= ~STATE_ALERTED;
                    self->chill = ENEMY1_CHILL;
                }
            }
            if(self->state & STATE_ALERTED){
                const int d2 = distance2(game.player.x / TILEW, game.player.y / TILEH, self->x / TILEW, self->y / TILEH);
                if(d2 < 2)
                    game.player.state &= ~STATE_CLEAN_ON_KILL;
                else if(self->weapon != WEAPON_NONE){
                    const int weapon_range = get_weapon_range(self->weapon);
                    if(d2 < weapon_range * weapon_range){
                        game.player.state &= ~STATE_CLEAN_ON_KILL;
                        self->state |= STATE_SHOOTING;
                    }
                }
            }
            self->targetx = game.player.x;
            self->targety = game.player.y;
        }
        else{
            if(self->x == self->targetx && self->y == self->targety){
                self->state &= ~STATE_ALERTED;
            }
            else if(!move_towards_visible(self, self->targetx, self->targety)){
                self->state &= ~STATE_ALERTED;
            }
        }
        
        return 0;
    }

    const int facing_player = facing_entity(self->x, self->y, self->orientation, game.player.x, game.player.y);

    if(!facing_player){
        if(!self->chill--){
            self->chill = ENEMY1_CHILL;
            self->orientation = (self->orientation + 1) % ORIENT_COUNT;
        }
        return 0;
    }

    const int can_see_player = is_target_visible_from(game.map, game.player.x, game.player.y, self->x, self->y, -1.0f);

    if(can_see_player){
        self->targetx = game.player.x;
        self->targety = game.player.y;
        self->state |= STATE_ALERTED;
        //self->sprite = SPRITE_ALERT;
        if((ABS(self->x - game.player.x) <= TILEW) && (ABS(self->y - game.player.y) <= TILEH)){
            game.player.state &= ~STATE_CLEAN_ON_KILL;
        }
    } else{
        if(--self->chill < 0){
            self->chill = ENEMY1_CHILL;
            self->orientation = (self->orientation + 1) % ORIENT_COUNT;
        }
        //self->sprite = SPRITE_ORIENTATION + self->orientation;
    }

    return 0;
}


#endif // =====================  END OF FILE ENTITIES_C ===========================