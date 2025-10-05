#include "renderer.h"
#include "sdl.h"


void draw_map(Game* game){
    if(!game->map.map) return;

    const int irange = 1 + game->camera.h / TILEH;
    const int jrange = 1 + game->camera.w / TILEW;
    for(int i = 0; i < irange; i+=1){
        for(int j = 0; j < jrange; j+=1){
            const char tile = get_tile(game->map, game->camera.x / TILEW + j, game->camera.y / TILEH + i);
            if(tile)
                fill_rect(
                    game->draw_canvas,
                    j * TILEW - (game->camera.x % TILEW),
                    i * TILEH - (game->camera.y % TILEH),
                    TILEW, TILEH,
                    (tile > 31)? 0xFF0000FF : palettes[PALETTE_MAP][tile]
                );
        }
    }
}

int general_check(){

    int output = 0;

    if(sizeof(palettes) / sizeof(*palettes) != PALETTE_COUNT){
        VERROR("palettes size (%zu) does not match expected (%i)", sizeof(palettes) / sizeof(*palettes), PALETTE_COUNT);
        output += 1;
    }

    return output;
}

int main(/*int argc, char** argv*/){

    if(general_check()){
        return 1;
    }

    static Game game;

    static Pixel draw_canvas_pixels[800 * 600];

    game.draw_canvas = create_surface(draw_canvas_pixels, 800, 600, 800);

    game.camera = (Rect){.x = 0, .y = 0, .w = 32 * TILEW, .h = 24 * TILEH};
    game.mouse  = (Rect){.x = 0, .y = 0, .w = TILEW / 4 , .h = TILEH / 4 };

    load_map(&game, map1, map1w, map1h);

    game.active = 1;

    const char* errmsg = "NO ERROR";

    if(init_subsystem(&game, &errmsg)){
        VERROR("%s", errmsg);
        return 1;
    }

    if(!game.user_draw){
        ERROR("missing game.user_draw callback");
        return 1;
    }
    if(!game.user_update){
        ERROR("missing game.user_update callback");
        return 1;
    }

    while(game.active){

        game.camera.x = (game.player.x + TILEW / 2) - game.camera.w / 2;
        game.camera.y = (game.player.y + TILEH / 2) - game.camera.h / 2;

        fill_rect(game.draw_canvas, 0, 0, game.camera.w, game.camera.h, 0xFF999999);

        draw_map(&game);

        fill_rect(game.draw_canvas, game.player.x - game.camera.x, game.player.y - game.camera.y, TILEW, TILEH, 0xFFAA1111);

        for(int i = 0; i < game.entity_count; ){
            if(!game.entities[i].update){
                for(int j = i + 1; j < game.entity_count; j+=1){
                    game.entities[j - 1] = game.entities[j];
                }
                game.entity_count -= 1;
                continue;
            }
            if(game.entities[i].update(&game, game.entities + i)){
                for(int j = i + 1; j < game.entity_count; j+=1){
                    game.entities[j - 1] = game.entities[j];
                }
                game.entity_count -= 1;
                continue;
            }
            i+=1;
        }

         // raycast
        Ray rayh = prepare_ray(
            game.player.x, game.player.y,
            game.mouse.x + game.camera.x, game.mouse.y + game.camera.y
        );
        Ray rayv = rayh;
        do{
            fill_rect(
                game.draw_canvas,
                (int) (rayh.x) - game.camera.x,
                (int) (rayh.y) - game.camera.y,
                TILEW / 4, TILEH / 4,
                0xFFAA00AA
            );
        } while(!ray_steph(game.map, &rayh) && rayh.r2 < 10 << 16);
        do{
            fill_rect(
                game.draw_canvas,
                (int) (rayv.x) - game.camera.x,
                (int) (rayv.y) - game.camera.y,
                TILEW / 4, TILEH / 4,
                0xFF00AAAA
            );
        } while(!ray_stepv(game.map, &rayv) && rayv.r2 < 10 << 16);
        

        fillrect(game.draw_canvas, game.mouse, 0xFF0011AA);

        game.debug = 0;
        game.user_update(&game);
        game.user_draw(&game);
    }


    if(quit_subsystem(&game, &errmsg)){
        VERROR("%s", errmsg);
        return 1;
    }

    return 0;
}
