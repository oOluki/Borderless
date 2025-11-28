#ifndef ASSETS_C
#define ASSETS_C

#include "assets.h"
#include "assets/font.c"
#include "assets/sprites.c"
#include "assets/map1.c"

int get_sprite_from_char(char c){
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

char get_char_from_sprite(int sprite){
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

#endif // =====================  END OF FILE ASSETS_C ===========================