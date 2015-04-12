#ifndef __NST_28037B_H__
#define __NST_28037B_H__

#include "gpio.h"

#define TOUCH_XM		PA0
#define TOUCH_XM_CH		ADC_Channel_0
#define TOUCH_YP		PA1
#define TOUCH_YP_CH		ADC_Channel_1
#define TOUCH_XP		PA2
#define TOUCH_XP_CH		ADC_Channel_2
#define TOUCH_YM		PA3
#define TOUCH_YM_CH		ADC_Channel_3

#define LCD_RD			PA4
#define LCD_CS			PA5
#define LCD_WR			PA6
#define LCD_CMD			PA7
#define LCD_RESET		PA8
#define LCD_BACKLIGHT	PA15

#define HOST_USART				USART1
#define HOST_USART_IRQn			USART1_IRQn
#define HOST_USART_IRQHandler	USART1_IRQHandler
#define HOST_TXD				PA9
#define HOST_RXD				PA10

// Convert r, g, b to RRRRRGGGGGGBBBBB
#define RGB565CONVERT(r,g,b)	((((r >> 3) & 0x1F) << 11) | (((g >> 2) & 0x3F) << 5) | (((b >> 3) & 0x1F) << 0))

typedef enum {
	ORIENT_0	= 0,
	ORIENT_1	= 1,
	ORIENT_2	= 2,
	ORIENT_3	= 3,
} TFT_ORIENTATION;

typedef enum {
	BLACK			= RGB565CONVERT(0, 0, 0),

	BLUE			= RGB565CONVERT(0, 0, 128),
	LIGHTBLUE		= RGB565CONVERT(128, 128, 255),
	BRIGHTBLUE		= RGB565CONVERT(0, 0, 255),

	GREEN			= RGB565CONVERT(0, 128, 0),
	LIGHTGREEN		= RGB565CONVERT(128, 255, 128),
	GREEN192		= RGB565CONVERT(0, 192, 0),
	GREEN224		= RGB565CONVERT(0, 224, 0),
	BRIGHTGREEN		= RGB565CONVERT(0, 255, 0),

	CYAN			= RGB565CONVERT(0, 128, 128),
	LIGHTCYAN		= RGB565CONVERT(128, 255, 255),
	BRIGHTCYAN		= RGB565CONVERT(0, 255, 255),

	RED				= RGB565CONVERT(128, 0, 0),
	LIGHTRED		= RGB565CONVERT(255, 128, 128),
	BRIGHTRED		= RGB565CONVERT(255, 0, 0),

	MAGENTA			= RGB565CONVERT(128, 0, 128),
	LIGHTMAGENTA	= RGB565CONVERT(255, 128, 255),
	BRIGHTMAGENTA	= RGB565CONVERT(255, 0, 255),

	YELLOW			= RGB565CONVERT(255, 255, 128),
	BRIGHTYELLOW 	= RGB565CONVERT(255, 255, 0),

	BROWN			= RGB565CONVERT(255, 128, 0),

	LIGHTGRAY		= RGB565CONVERT(128, 128, 128),
	DARKGRAY		= RGB565CONVERT(64, 64, 64),

	WHITE			= RGB565CONVERT(255, 255, 255),
	GRAY0			= RGB565CONVERT(224, 224, 224),
	GRAY1			= RGB565CONVERT(192, 192, 192),
	GRAY2			= RGB565CONVERT(160, 160, 160),
	GRAY3			= RGB565CONVERT(128, 128, 128),
	GRAY4			= RGB565CONVERT(96, 96, 96),
	GRAY5			= RGB565CONVERT(64, 64, 64),
	GRAY6			= RGB565CONVERT(32, 32, 32)

} COLOR;

void DisplayInit	(void);
void DisplayIdleApps(void);

int Height	(void);
int	Width	(void);

void PutLine		(const char * text);
void DisplayChar	(char c);
void DisplayCharAt	(uint8_t col, uint8_t row, char c);
int	 PutCharXY		(int x, int y, char c);

void DisplayAt		(uint8_t col, uint8_t row);
void DisplayTextAt	(uint8_t column, uint8_t row, const char * text);
int	 DisplayTextXY	(int x, int y, const char * c);
void DisplayText	(const char * c);
void DisplayTextLine(const char * c);

void SetNormalMode	(void);
void SetPartialMode	(uint16_t start, uint16_t end);
void SetScrollMode	(uint16_t top, uint16_t bottom);

void SetBackground	(COLOR color);
void SetForeground	(COLOR color);
COLOR ColorFromRGB	(uint8_t R, uint8_t G, uint8_t B);

void SetFontScale	(uint8_t scale);

void ClearScreen		(void);
void ClearScreenColor	(COLOR color);

void Pixel		(int x, int y, COLOR color);
void Circle		(int x0, int y0, int r, COLOR color);
void CircleFill	(int x, int y, int r, COLOR color);
void LineH		(int x0, int x1, int y, COLOR color);
void LineV		(int x, int y0, int y1, COLOR color);
void Rect		(int x0, int y0, int x1, int y1, COLOR color);
void RectFill	(int x, int y, int w, int h, COLOR color);

#endif
