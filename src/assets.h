#ifndef ASSETS_HEADER
#define ASSETS_HEADER

#define FONT_SIZE 8
#define FONT_ELEMENTS_PER_ROW 8
#define FONT_STRIDE (FONT_SIZE * FONT_ELEMENTS_PER_ROW)

enum FontSprites{
	FONT_0 = 0,
	FONT_9 = FONT_0 + 9,
	FONT_a,
	FONT_z = FONT_a + 25,

	FONT_COMMA,
	FONT_DOT,
	FONT_EXCLAMATION,
	FONT_INTERROGATION,
	FONT_OPEN_QUOTES,
	FONT_CLOSE_QUOTES,
	FONT_COLUMN,
	FONT_SEMI_COLUMN,
	FONT_OPENPAREN,
	FONT_CLOSEPAREN,
	FONT_PERCENT,
	FONT_SPACE,
	FONT_CURSOR,
	FONT_CURSOR_HOVERING,
	FONT_EMPTY0,
	FONT_EMPTY1,
	FONT_EMPTY2,
	FONT_EMPTY3,
	FONT_EMPTY4,
	FONT_EMPTY5,
	FONT_ORIENTATION,
    FONT_UP = FONT_ORIENTATION,
    FONT_RIGHT,
    FONT_DOWN,
    FONT_LEFT,
    FONT_DEAD,
	FONT_ALERT,

    // for counting purposes
    FONT_COUNT
};

enum EntitySprites{
	ENTITY_SPRITE_UP = 0,
	ENTITY_SPRITE_RIGHT,
	ENTITY_SPRITE_DOWN,
	ENTITY_SPRITE_LEFT,
	ENTITY_SPRITE_DEAD,
};

extern const char simple_console_font[];

extern const int  fontsheetw;
extern const int  fontsheeth;
extern const unsigned char fontsheet[];

extern const int entity_sprite_size;
extern const int entity_spritesheet_stride;
extern const unsigned char entity_spritesheet[];

extern const int map1w;
extern const int map1h;
extern const unsigned char map1[];

int get_sprite_from_char(char c);

char get_char_from_sprite(int sprite);

#endif // =====================  END OF FILE ASSETS_HEADER ===========================