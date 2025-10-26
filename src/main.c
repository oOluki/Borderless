#include "game.c"
#include "sdl.c"


int main(int argc, char** argv){

    static Pixel draw_canvas_pixels[800 * 600];

    if(game_init(draw_canvas_pixels, 800, 600, 800)) return 1;

    if(init_subsystem()) return 1;

    game.update(CMD_UPDATE);

    while(game.active){

        for(int cmd = get_cmd(); cmd != CMD_FINNISHED; cmd = get_cmd()){
            if(game.update(cmd)) break;
        }
        update_subsystem();
    }

    const char* errmsg = NULL;

    if(close_subsystem()){
        VERROR("%s", errmsg);
        return 1;
    }

    return 0;
}
