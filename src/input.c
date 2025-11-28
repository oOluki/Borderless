#ifndef INPUT_C
#define INPUT_C

#include "begin.h"
#include "input.h"

char get_cmd_char(int cmd){
    switch (cmd)
    {
    case CMD_NONE:              return ' ';
    case CMD_QUIT:              return 'q';
    case CMD_UPDATE:            return 'u';
    case CMD_DISPLAY:           return 'r';
    case CMD_DEBUG:             return 'D';
    case CMD_BACK:              return 'b';
    case CMD_ENTER:             return 'e';
    case CMD_TOGGLE:            return 'p';
    case CMD_MOUSECLICK:        return 'c';
    case CMD_UP:                return 'w';
    case CMD_RIGHT:             return 'd';
    case CMD_LEFT:              return 'a';
    case CMD_DOWN:              return 's';
    case CMD_CHEAT_RESTART:     return '@';
    case CMD_SPECIAL_SIGNAL:    return '^';
    case CMD_FINNISHED:         return '\n';
    default:                    return '?';
    }
}

int get_char_cmd(int _char){
    switch (_char)
    {
    case '\t':
    case ' ':   return CMD_NONE;
    case 'q':   return CMD_QUIT;
    case 'r':   return CMD_DISPLAY;
    case 'u':   return CMD_UPDATE;
    case 'D':   return CMD_DEBUG;
    case 'b':   return CMD_BACK;
    case 'e':   return CMD_ENTER;
    case 'p':   return CMD_TOGGLE;
    case 'c':   return CMD_MOUSECLICK;
    case 'w':   return CMD_UP;
    case 'd':   return CMD_RIGHT;
    case 'a':   return CMD_LEFT;
    case 's':   return CMD_DOWN;
    case '@':   return CMD_CHEAT_RESTART;
    case '^':   return CMD_SPECIAL_SIGNAL;
    case EOF:
    case '\0':
    case '\n':  return CMD_FINNISHED;
    default:    return CMD_ERROR;
    }
}

const char* get_cmd_str(int cmd){
    switch (cmd)
    {
    case CMD_QUIT:              return "CMD_QUIT";
    case CMD_UPDATE:            return "CMD_UPDATE";
    case CMD_DISPLAY:           return "CMD_DISPLAY";
    case CMD_DEBUG:             return "CMD_DEBUG";
    case CMD_BACK:              return "CMD_BACK";
    case CMD_ENTER:             return "CMD_ENTER";
    case CMD_TOGGLE:            return "CMD_TOGGLE";
    case CMD_MOUSECLICK:        return "CMD_MOUSECLICK";
    case CMD_UP:                return "CMD_UP";
    case CMD_RIGHT:             return "CMD_RIGHT";
    case CMD_LEFT:              return "CMD_LEFT";
    case CMD_DOWN:              return "CMD_DOWN";
    case CMD_CHEAT_RESTART:     return "CMD_CHEAT_RESTART";
    case CMD_SPECIAL_SIGNAL:    return "CMD_SPECIAL_SIGNAL";
    case CMD_FINNISHED:         return "CMD_FINNISHED";
    default:                    return "CMD_NONE";
    }
}

#endif // =====================  END OF FILE INPUT_C ===========================