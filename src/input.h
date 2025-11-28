#ifndef INPUT_HEADER
#define INPUT_HEADER

enum Cmd{
    CMD_NONE = 0,
    
    CMD_QUIT,
    CMD_UPDATE,
    CMD_DISPLAY,
    CMD_DEBUG,

    CMD_BACK,
    CMD_ENTER,
    CMD_TOGGLE,
    CMD_MOUSECLICK,

    CMD_UP,
    CMD_RIGHT,
    CMD_LEFT,
    CMD_DOWN,

    CMD_CHEAT_RESTART,

    // for external user usage
    CMD_SPECIAL_SIGNAL,

    // indicates last command in command queue
    CMD_FINNISHED,

    // for counting purposes
    CMD_COUNT,

    CMD_ERROR
};

char get_cmd_char(int cmd);

int get_char_cmd(int _char);

const char* get_cmd_str(int cmd);

#endif // =====================  END OF FILE INPUT_HEADER ===========================