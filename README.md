# Borderless

TODO:

    add colorfull characters, example usage:
    printf("\x1b[33mWarning!\x1b[0m\n");
    printf("\x1b[91mBright red\x1b[0m\n");
    printf("\x1b[96mBright cyan\x1b[0m\n");
    printf("\x1b[42mGreen background\x1b[0m\n");
    printf("\x1b[44mBlue background\x1b[0m\n");
    printf("\x1b[38;5;202mOrange-ish text\x1b[0m\n");
    printf("\x1b[38;2;2;5;0mCustom RGB\x1b[0m\n");


Signals:

to draw the game at any point use the signals (where each * is a signal CMD_SPECIAL_SIGNAL command and r is the CMD_DISPLAY command):
    ***r -> clears all marks (in the whole map)
    **r  -> changes draw_mode to graphical, if draw_mode is not graphical, or to none otherwise
    *r   -> changes draw_mode to console, if draw_mode is not console, or to none otherwise
    you can combine this to achieve any pattern, for instance:
    **r *r     -> takes any draw_mode to console mode
    **r *r *r  -> takes any draw_mode to none
    **r *r **r -> takes any draw_mode to graphical





simple 2d game...

FOR NOW
