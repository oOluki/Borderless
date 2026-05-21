# Borderless

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
