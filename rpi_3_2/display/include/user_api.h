#ifndef USER_API_H
#define USER_API_H

//>>
// Size
#define LCD_WIDTH 320
#define LCD_HEIGHT 240

// Colors
#define COLOR_BLACK 0x0000
#define COLOR_BLUE 0x001F
#define COLOR_RED 0xF800
#define COLOR_GREEN 0x07E0
#define COLOR_CYAN 0x07FF
#define COLOR_MAGENTA 0xF81F
#define COLOR_YELLOW 0xFFE0
#define COLOR_WHITE 0xFFFF
#define COLOR_COLOR565(r, g, b)                                                \
	(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3))

// User commands
enum lcd_commands {
	NONE,
	RENDER,
	SET_WINDOW,
	DRAW_RECTANGLE,
	DRAW_DISPLAY,
	DRAW_STRING,
	DRAW_BALL,
};
//>>

#endif //USER_API_H
