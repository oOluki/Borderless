#ifndef SPRITES_HEADER
#define SPRITES_HEADER

#include "font.h"


static inline char get_char_from_sprite(int sprite){
	switch (sprite)
	{
	case FONT_COMMA:	 		return ',';
	case FONT_DOT:	 			return '.';
	case FONT_EXCLAMATION:		return '!';
	case FONT_INTERROGATION:	return '?';
	case FONT_OPEN_QUOTES:		return '\"';
	case FONT_CLOSE_QUOTES:		return '\"';
	case FONT_COLUMN:	 		return ':';
	case FONT_SEMI_COLUMN:		return ';';
	case FONT_OPENPAREN:	 	return '[';
	case FONT_CLOSEPAREN:	 	return ']';
	case FONT_PERCENT:	 		return '%';
	case FONT_SPACE:	 		return ' ';
	default:
		return (sprite >= FONT_0 && sprite <= FONT_9)? (char) ('0' + sprite - FONT_0) : 
		((sprite >= FONT_a && sprite <= FONT_z)? (char) ('a' + sprite - FONT_a) : '\0');
	}
}

static inline int get_sprite_from_char(char c){
	switch (c)
	{
	case ',':	return FONT_COMMA;
	case '.':	return FONT_DOT;
	case '!':	return FONT_EXCLAMATION;
	case '?':	return FONT_INTERROGATION;
	case '\"':	return FONT_OPEN_QUOTES;
	case ':':	return FONT_COLUMN;
	case ';':	return FONT_SEMI_COLUMN;
	case '[':	return FONT_OPENPAREN;
	case ']':	return FONT_CLOSEPAREN;
	case '%':	return FONT_PERCENT;
	case ' ':	return FONT_SPACE;
	default:
		if(c >= '0' && c <= '9') return (int) (FONT_0 + c - '0');
		if(c >= 'a' && c <= 'z') return (int) (FONT_a + c - 'a');
		if(c >= 'A' && c <= 'Z') return (int) (FONT_a + c - 'A');
		return -1;
	}
}

static const char simple_console_font[] = {
	'0', '1', '2', '3', '4', '5', '6', '7',
	'8', '9', 'a', 'b', 'c', 'd', 'e', 'f',
	'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
	'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
	'w', 'x', 'y', 'z', ',', '.', '!', '?',
	'"', '"', ':', ';', '[', ']', '%', ' ',
	'*', '~', ' ', ' ', ' ', ' ', ' ', ' ',
	'^', '>', 'V', '<', 'T', '!', ' ', ' '
};

enum EntitySprites{
	ENTITY_SPRITE_UP = 0,
	ENTITY_SPRITE_RIGHT,
	ENTITY_SPRITE_DOWN,
	ENTITY_SPRITE_LEFT,
	ENTITY_SPRITE_DEAD,
};

static const int entity_sprite_size = 10;
static const int entity_spritesheet_stride = 10;
static const char entity_spritesheet[] = {

    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 2, 2, 2, 2, 2, 2, 2, 2, 1,
    1, 2, 1, 1, 2, 2, 1, 1, 2, 1,
    1, 2, 1, 1, 2, 2, 1, 1, 2, 1,
    1, 2, 1, 1, 2, 2, 1, 1, 2, 1,
    1, 2, 2, 2, 2, 2, 2, 2, 2, 1,
    1, 2, 2, 2, 2, 2, 2, 2, 2, 1,
    1, 2, 2, 2, 2, 2, 2, 2, 2, 1,
    1, 2, 2, 2, 2, 2, 2, 2, 2, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,

	1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 2, 2, 2, 2, 2, 2, 2, 2, 1,
	1, 2, 2, 2, 2, 1, 1, 1, 2, 1,
	1, 2, 2, 2, 2, 1, 1, 1, 2, 1,
	1, 2, 2, 2, 2, 2, 2, 2, 2, 1,
	1, 2, 2, 2, 2, 2, 2, 2, 2, 1,
	1, 2, 2, 2, 2, 1, 1, 1, 2, 1,
	1, 2, 2, 2, 2, 1, 1, 1, 2, 1,
	1, 2, 2, 2, 2, 2, 2, 2, 2, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1,

    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 2, 2, 2, 2, 2, 2, 2, 2, 1,
    1, 2, 2, 2, 2, 2, 2, 2, 2, 1,
    1, 2, 2, 2, 2, 2, 2, 2, 2, 1,
    1, 2, 2, 2, 2, 2, 2, 2, 2, 1,
    1, 2, 1, 1, 2, 2, 1, 1, 2, 1,
    1, 2, 1, 1, 2, 2, 1, 1, 2, 1,
    1, 2, 1, 1, 2, 2, 1, 1, 2, 1,
    1, 2, 2, 2, 2, 2, 2, 2, 2, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,

	1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 2, 2, 2, 2, 2, 2, 2, 2, 1,
	1, 2, 1, 1, 1, 2, 2, 2, 2, 1,
	1, 2, 1, 1, 1, 2, 2, 2, 2, 1,
	1, 2, 2, 2, 2, 2, 2, 2, 2, 1,
	1, 2, 2, 2, 2, 2, 2, 2, 2, 1,
	1, 2, 1, 1, 1, 2, 2, 2, 2, 1,
	1, 2, 1, 1, 1, 2, 2, 2, 2, 1,
	1, 2, 2, 2, 2, 2, 2, 2, 2, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1,

	1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 2, 2, 2, 2, 2, 2, 2, 2, 1,
	1, 2, 1, 2, 2, 2, 2, 1, 2, 1,
	1, 2, 2, 1, 2, 2, 1, 2, 2, 1,
	1, 2, 2, 2, 1, 1, 2, 2, 2, 1,
	1, 2, 2, 2, 1, 1, 2, 2, 2, 1,
	1, 2, 2, 1, 2, 2, 1, 2, 2, 1,
	1, 2, 1, 2, 2, 2, 2, 1, 2, 1,
	1, 2, 2, 2, 2, 2, 2, 2, 2, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};

static const int pistol_spritew = 4;
static const int pistol_spriteh = 3;
static const char pistol_sprite_up[] = {
	1, 0, 0,
	1, 0, 0,
	1, 1, 0,
	1, 1, 1,
};
static const char pistol_sprite_right[] = {
	1, 1, 1, 1,
	1, 1, 0, 0,
	1, 0, 0, 0,
};
static const char pistol_sprite_down[] = {
	1, 1, 1,
	0, 1, 1,
	0, 0, 1,
	0, 0, 1,
};
static const char pistol_sprite_left[] = {
	0, 0, 0, 1,
	0, 0, 1, 1,
	1, 1, 1, 1,
};

#endif // =====================  END OF FILE SPRITES_HEADER ===========================