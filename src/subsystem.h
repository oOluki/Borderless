#ifndef SUBSYSTEM_HEADER
#define SUBSYSTEM_HEADER

void report();

int initascii_subsystem();

int closeascii_subsystem();

int updateascii_subsystem();

int getascii_cmd();

#ifdef SUPPORT_SDL

    int initsdl_subsystem();

    int closesdl_subsystem();

    int getsdl_cmd();

    int updatesdl_subsystem();

#endif // END OF #ifdef SUPPORT_SDL

#endif // =====================  END OF FILE SUBSYSTEM_HEADER ===========================