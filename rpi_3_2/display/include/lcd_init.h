#ifndef __LCD_INIT_H__
#define __LCD_INIT_H__

#include <linux/kernel.h>
#include "user_api.h"

#define LCD_PIN_CS 7
#define LCD_PIN_RESET 27
#define LCD_PIN_DC 22

#define ILI9341_MADCTL_MY 0x80
#define ILI9341_MADCTL_MX 0x40
#define ILI9341_MADCTL_MV 0x20
#define ILI9341_MADCTL_ML 0x10
#define ILI9341_MADCTL_RGB 0x00
#define ILI9341_MADCTL_BGR 0x08
#define ILI9341_MADCTL_MH 0x04

// default orientation
#define LCD_ROTATION (ILI9341_MADCTL_MX | ILI9341_MADCTL_BGR)

#define LCD_CASET 0x2A
#define LCD_RASET 0x2B
#define LCD_RAMWR 0x2C

typedef u16 pixel_t;

typedef struct {
	size_t size;
	u16 n_rows;
	u16 n_cols;
	pixel_t *buffer;
	pixel_t unraveled[LCD_WIDTH * LCD_HEIGHT];
} lcd_frame;

struct window {
	u16 x0;
	u16 x1;
	u16 y0;
	u16 y1;
	u16 w;
	u16 h;
};

typedef struct {
	u8 cmd;
	u32 delay;
	u8 data[];
} init_step;

#endif //__LCD_INIT_H__
