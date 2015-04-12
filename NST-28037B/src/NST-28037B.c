#include "gpio.h"
#include "NST-28037B.h"
#include "main.h"
#include <stdio.h>

#define CURSOR_DELAY		500

#define		REG_01			0x01	/* Display Mode Control	*/
	#define	REG_01_PLTON	0x01
	#define	REG_01_INVON	0x02
	#define	REG_01_IDMON	0x04
	#define	REG_01_SCROLL	0x08
	#define	REG_01_DP_STB_S	0x40
	#define	REG_01_DP_STB	0x80

#define		REG_02			0x02
#define		REG_04			0x04
#define		REG_06			0x06
#define		REG_08			0x08

#define		REG_TFA			0x0E	/* Vertical scroll top fixed area register		*/
#define		REG_VSA			0x10	/* Vertical scroll height area register			*/
#define		REG_BFA			0x12	/* Vertical scroll button fixed area register	*/
#define		REG_VSP			0x14	/* Vertical scroll start address register		*/

#define		REG_16			0x16	/* Memory Access Control	*/
	#define	REG_16_BGR			0x08
	#define	REG_16_ML			0x10
	#define	REG_16_MV			0x20
	#define	REG_16_MX			0x40
	#define	REG_16_MY			0x80
#define		REG_17				0x17	/* COLMOD Control Register	*/
	#define	REG_17_IFPF			0x07
	#define	REG_17_IFPF_16BBP	0x05
	#define	REG_17_CSEL			0xF0
	#define	REG_17_CSEL_16BBP	0x00
#define		REG_18			0x18	/* OSC Control Register		*/
	#define	REG_18_NP_RADJ		0x0F
	#define	REG_18_IP_RADJ		0xF0
#define		REG_19			0x19	/* OSC Control Register 2	*/
	#define	REG_19_OSCEN		0x01
#define		REG_1A			0x1A
#define		REG_1B			0x1B
#define		REG_1F			0x1F	/* Power Control 6			*/
	#define	REG_1F_STB		0x01
	#define	REG_1F_DDVDH_TRI	0x02
	#define	REG_1F_XDK			0x04
	#define	REG_1F_DK			0x08
	#define	REG_1F_PON			0x10
	#define	REG_1F_VCOMG		0x40
	#define	REG_1F_GASEN		0x80
#define		REG_23			0x23
#define		REG_24			0x24
#define		REG_25			0x25
#define		REG_27			0x27	/* Display Control 2 Register	*/
	#define	REG_27_REF			0x01
	#define	REG_27_PTG			0x02
	#define	REG_27_PTV0			0x10
	#define	REG_27_PTV1			0x20
	#define	REG_27_PT0			0x40
	#define	REG_27_PT1			0x80
#define		REG_28			0x28	/* Display Control 3 Register	*/
	#define	REG_28_D0			0x04
	#define	REG_28_D1			0x08
	#define	REG_28_DTE			0x10
	#define	REG_28_GON			0x20
#define		REG_36			0x36	/* Panel Characteristic Control Register */
	#define	REG_36_BGR_PANEL	0x01
	#define	REG_36_REV_PANEL	0x02
	#define	REG_36_GS_PANEL		0x04
	#define	REG_36_SS_PANEL		0x08
#define		REG_E8			0xE8	/* Source OP control	*/
#define		REG_E9			0xE9	/* Source OP control	*/

#define		REG_EA			0xEA	/* Power Control Internal Used (1)	*/
#define		REG_EB			0xEB	/* Power Control Internal Used (2)	*/
#define		REG_EC			0xEC	/* Source Control Internal Used (1)	*/
#define		REG_ED			0xED	/* Source Control Internal Used (2)	*/

#define		REG_F1			0xF1
#define		REG_F2			0xF2

#define		REG_22			0x22	

enum {
	TOUCH_XM_START		= 0,
	TOUCH_XP_START		= 1,
	TOUCH_YM_START		= 2,
	TOUCH_YP_START		= 3,
	TOUCH_PRESS_1		= 4,
	TOUCH_PRESS_2		= 5,
	TOUCH_END			= 6
};

#define TOUCH_SAMPLE	ADC_SampleTime_239Cycles5
// ADC_SampleTime_55Cycles5

extern const uint8_t	Font8x18_Consolas[];
extern const uint8_t	Font8x16_System[];
extern const uint8_t	Font8x8_Small[];
extern const uint8_t	Font8x16_Gothic[];

extern uint16_t	CursorTimer;
extern uint8_t	AdcStartDelay;

typedef struct LCD_s {
	const uint8_t * FontTable;
		COLOR		ColorFG, ColorBG;
		uint8_t		TextCol, TextRow, CursorState;
		uint8_t		Orientation, FontScale, AutoScroll, AutoWrap;
		uint16_t	ScrollAddr;
		uint8_t		TouchState;
		uint8_t		TouchIdx;
		uint16_t	TouchXP,
					TouchXM,
					TouchYP,
					TouchYM,
					TouchZ1,
					TouchZ2;
		uint16_t	TouchXMA[20], TouchYMA[20], TouchXPA[20], TouchYPA[20];
} LCD_t;

static LCD_t LCD;

const uint8_t InitRegs[] = {
		REG_EA, 0x00,	// Reset Power Control 1
		REG_EB, 0x20,	// Power Control 2
		REG_EC, 0x0C,	// Power Control 3
		REG_ED, 0xC4,	// Power Control 4
		REG_E8, 0x40,	// Source OPON_N
		REG_E9, 0x38,	// Source OPON_I
		REG_F1, 0x01,
		REG_F2, 0x10,
		REG_27, 0xA3,	// (REG_27_PT1 | REG_27_PTV1 | REG_27_PTG | REG_27_REF),

	/* Gamma settings  -----------------------------------------------------------*/
		0x40,0x00,
		0x41,0x00,
		0x42,0x01,
		0x43,0x13,
		0x44,0x10,
		0x45,0x26,
		0x46,0x08,
		0x47,0x51,
		0x48,0x02,
		0x49,0x12,
		0x4A,0x18,
		0x4B,0x19,
		0x4C,0x14,
		
		0x50,0x19,
		0x51,0x2F,
		0x52,0x2C,
		0x53,0x3E,
		0x54,0x3F,
		0x55,0x3F,
		0x56,0x2E,
		0x57,0x77,
		0x58,0x0B,
		0x59,0x06,
		0x5A,0x07,
		0x5B,0x0D,
		0x5C,0x1D,
		0x5D,0xCC,

	/* Power On sequence ---------------------------------------------------------*/
		REG_1B, 0x1B,	// Power Control 2
		REG_1A, 0x01,	// Power Control 1
		REG_24, 0x2F,	// Vcom Control 2
		REG_25, 0x57,	// Vcom Control 3

		REG_23, 0x8D,	// Vcom Control 1

	/* Power + Osc ---------------------------------------------------------------*/
		REG_18, 0x36,			// Normal/Partial freq. = 75MHz, Idle freq. 55MHz
		REG_19, REG_19_OSCEN,	// Enable OSC
		REG_01, 0x00,
		REG_1F, (REG_1F_GASEN | REG_1F_DK),
		0, 5,			// Delay 5 ms
		REG_1F, (REG_1F_GASEN),
		0, 5, 			// Delay 5 ms
		REG_1F, (REG_1F_GASEN | REG_1F_PON),
		0, 5,			// Delay 5 ms
		REG_1F, (REG_1F_GASEN | REG_1F_VCOMG | REG_1F_PON),
		0, 5,			// Delay 5 ms
		REG_17, 0x05,	// (REG_17_CSEL_16BBP | REG_17_IFPF_16BBP),	// COLMOD 16Bit/Pixel
		REG_36, 0x00,	// Panel Characteristic
		REG_28, 0x38,	//(REG_28_GON | REG_28_DTE | REG_28_D1),
		0, 40,			// Delay 40 ms
		REG_28, 0x3C,	// (REG_28_GON | REG_28_DTE | REG_28_D1 | REG_28_D0),
		0, 0
};

void LCD_BL_HIGH()	{	GPIOA->BSRR = (1 << LCD_BACKLIGHT);		}
void LCD_BL_LOW()	{	GPIOA->BRR  = (1 << LCD_BACKLIGHT);		}
void LCD_RST_LOW()	{	GPIOA->BRR  = (1 << LCD_RESET);			}
void LCD_RST_HIGH()	{	GPIOA->BSRR = (1 << LCD_RESET);			}
void LCD_RD_LOW()	{	GPIOA->BRR  = (1 << LCD_RD);			}
void LCD_RD_HIGH()	{	GPIOA->BSRR = (1 << LCD_RD);			}
void LCD_CS_LOW()	{	GPIOA->BRR  = (1 << LCD_CS);			}
void LCD_CS_HIGH()	{	GPIOA->BSRR = (1 << LCD_CS);			}
void LCD_WR_LOW()	{	GPIOA->BRR  = (1 << LCD_WR);			}
void LCD_WR_HIGH()	{	GPIOA->BSRR = (1 << LCD_WR);			}
void LCD_CMD_LOW()	{	GPIOA->BRR  = (1 << LCD_CMD);			}
void LCD_CMD_HIGH()	{	GPIOA->BSRR = (1 << LCD_CMD);			}
void LCD_DATA_OUT()	{	GPIOB->CRL = GPIOB->CRH = 0x22222222;	}
void LCD_DATA_IN()	{	GPIOB->CRL = GPIOB->CRH = 0x44444444;	}

void	LcdWriteReg		(uint8_t reg, uint8_t data);
void	LcdWriteReg16	(uint8_t reg, uint16_t addr);
uint8_t	LcdReadReg		(uint8_t reg);

/**********************************************************************
 *	Write data to LCD
 *********************************************************************/
#if 0
	void LcdWrite(uint16_t data)
	{
		GPIOB->ODR = data;
		LCD_WR_LOW();
		LCD_WR_HIGH();
	}
#else
	#define LcdWrite(data)		\
		do {					\
			GPIOB->ODR = data;	\
			LCD_WR_LOW();		\
			LCD_WR_HIGH();		\
		} while (0)
#endif

/**********************************************************************
 *	Read data from LCD
 *********************************************************************/
uint16_t LcdRead()
{
	uint16_t data;

	LCD_DATA_IN();
	LCD_RD_LOW();
	__NOP();
	data = GPIOB->IDR;
	LCD_RD_HIGH();
	LCD_DATA_OUT();
	return data;
}

/**********************************************************************
 *	Write command to LCD
 *********************************************************************/
#if 0
	void LcdWriteCommand(uint8_t cmd)
	{
		LCD_CMD_LOW();
		LcdWrite(cmd);
		LCD_CMD_HIGH();
	}
#else
	#define LcdWriteCommand(cmd)	\
		do {						\
			LCD_CMD_LOW();			\
			LcdWrite(cmd);			\
			LCD_CMD_HIGH();			\
		} while (0)
#endif

/**********************************************************************
 *	Write data to LCD register
 *********************************************************************/
#if 0
	void LcdWriteReg(uint8_t cmd, uint8_t data)
	{
		LcdWriteCommand(cmd);
		LcdWrite(data);
	}
#else
	#define LcdWriteReg(cmd, data)	\
		do {						\
			LcdWriteCommand(cmd);	\
			LcdWrite(data);			\
		} while (0)
#endif

/**********************************************************************
 *	Read data from LCD register
 *********************************************************************/
uint8_t LcdReadReg(uint8_t reg)
{
	uint16_t data;

	LcdWriteCommand(reg);
	data = (LcdRead() & 0xFF);
	return data;
}

/**********************************************************************
 *	Write to LCD REG 22
 *********************************************************************/
#define LcdWriteCommand22()		LcdWriteCommand(REG_22)

/**********************************************************************
 *	Write 16 bit to LCD register
 *********************************************************************/
#define LcdWriteReg16(reg, addr)			\
	do {									\
		LcdWriteReg (reg + 1, (addr) );		\
		LcdWriteReg (reg + 0, (addr) >> 8);	\
	} while (0)

/**********************************************************************
 *	Set LCD GRAM address
 *********************************************************************/
void LcdSetAddress (uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
	LcdWriteReg16 (REG_02, x);
	LcdWriteReg16 (REG_04, x + w - 1);

	LcdWriteReg16 (REG_06, y);
	LcdWriteReg16 (REG_08, y + h - 1);
}

/**********************************************************************
 *	Set LCD scroll
 *********************************************************************/
void LcdSetScroll()
{
	LCD_CS_LOW();
	LcdWriteReg16(REG_VSP, LCD.ScrollAddr);
	LCD_CS_HIGH();
}

/**********************************************************************
 *	Set LCD scroll to line
 *********************************************************************/
void LcdScrollTo(int lines)
{
	int h = Height();
	if (lines > 0)
		LCD.ScrollAddr += lines;
	else
		LCD.ScrollAddr += (h + lines);
	LCD.ScrollAddr %= h;
	LcdSetScroll();
}

/**********************************************************************
 *	Set scroll mode
 *********************************************************************/
void SetScrollMode(uint16_t top, uint16_t bottom)
{
	LCD_CS_LOW();

	LcdWriteReg16(REG_TFA, top);
	LcdWriteReg16(REG_VSA, Height() - top - bottom);
	LcdWriteReg16(REG_BFA, bottom);
	LcdWriteReg16(REG_VSP, top);
	LCD.ScrollAddr = top;

	LcdWriteReg(REG_01, (LcdReadReg(REG_01) | REG_01_SCROLL));

	LCD_CS_HIGH();
}

/**********************************************************************
 *	Set normal mode
 *********************************************************************/
void SetNormalMode()
{
	LCD_CS_LOW();
	LcdWriteReg(REG_01, (LcdReadReg(REG_01) & ~(REG_01_SCROLL | REG_01_PLTON)));
	LCD.ScrollAddr = 0;
	LcdSetScroll();
	LCD_CS_HIGH();
}

/**********************************************************************
 *	Move to next text line
 *********************************************************************/
void LcdNextLine(void)
{
	uint8_t row_height = LCD.FontTable[1] * LCD.FontScale;
	int h = Height();

	LCD.TextRow++;
	if (LCD.AutoScroll != 0 && LCD.Orientation == ORIENT_0)
	{
		if (LCD.TextRow * row_height >= (LCD.ScrollAddr == 0 ? h : LCD.ScrollAddr))
		{
			LCD.TextRow--;
			LcdScrollTo(row_height);
			if (LCD.ScrollAddr >= row_height)
				RectFill(0, LCD.ScrollAddr - row_height, Width(), row_height, LCD.ColorBG);
			else
				RectFill(0, LCD.ScrollAddr - row_height + h, Width(), row_height, LCD.ColorBG);
		}
	}
	DisplayAt(0, LCD.TextRow);
}

/**********************************************************************
 *	Get screen width
 *********************************************************************/
int Width()
{
	if (LCD.Orientation == ORIENT_0 || LCD.Orientation == ORIENT_2) return 240;
	else return 320;
}

/**********************************************************************
 *	Get screen height
 *********************************************************************/
int Height()
{
	if (LCD.Orientation == ORIENT_0 || LCD.Orientation == ORIENT_2) return 320;
	else return 240;
}

/**********************************************************************
 *	Set orientation
 *********************************************************************/
void SetOrientation(TFT_ORIENTATION o)
{
	LCD.Orientation = o;
	LCD_CS_LOW();
	switch (LCD.Orientation)
	{
		case ORIENT_0:
			LcdWriteReg(REG_16, REG_16_BGR);
			break;
		case ORIENT_1:
			LcdWriteReg(REG_16, (REG_16_MX | REG_16_MV | REG_16_BGR));
			break;
		case ORIENT_2:
			LcdWriteReg(REG_16, (REG_16_MY | REG_16_MX | REG_16_BGR));
			break;
		case ORIENT_3:
			LcdWriteReg(REG_16, (REG_16_MY | REG_16_MV | REG_16_BGR));
			break;
	}
	LCD_CS_HIGH();
}

/**********************************************************************
 *	Set touch to HiZ
 *********************************************************************/
void SelectTouchNone()
{
	pinMode(TOUCH_XM, PinMode_AIn);
	pinMode(TOUCH_XP, PinMode_AIn);
	pinMode(TOUCH_YP, PinMode_AIn);
	pinMode(TOUCH_YM, PinMode_AIn);
}

/**********************************************************************
 *	Prepare ADC for Touch X
 *********************************************************************/
void SelectTouchPress()
{
	pinMode(TOUCH_XM, PinMode_AIn);
	pinMode(TOUCH_YP, PinMode_AIn);
	if (LCD.Orientation == ORIENT_0 || LCD.Orientation == ORIENT_1)
	{
		pinLow(TOUCH_XP);
		pinHigh(TOUCH_YM);
	}
	else
	{
		pinHigh(TOUCH_XP);
		pinLow(TOUCH_YM);
	}
	pinMode(TOUCH_XP, PinMode_Out_PP);
	pinMode(TOUCH_YM, PinMode_Out_PP);
	ADC_RegularChannelConfig(ADC1, TOUCH_XM_CH, 1, TOUCH_SAMPLE);
}

/**********************************************************************
 *	Prepare ADC for Touch X
 *********************************************************************/
void SelectTouchX()
{
	pinMode(TOUCH_XM, PinMode_AIn);
	pinMode(TOUCH_XP, PinMode_AIn);
	if (LCD.Orientation == ORIENT_0 || LCD.Orientation == ORIENT_1)
	{
		pinHigh(TOUCH_YP);
		pinLow(TOUCH_YM);
	}
	else
	{
		pinLow(TOUCH_YP);
		pinHigh(TOUCH_YM);
	}
	pinMode(TOUCH_YP, PinMode_Out_PP);
	pinMode(TOUCH_YM, PinMode_Out_PP);
	ADC_RegularChannelConfig(ADC1, TOUCH_XM_CH, 1, TOUCH_SAMPLE);
}

/**********************************************************************
 *	Prepare ADC for Touch Y
 *********************************************************************/
void SelectTouchY()
{
	pinMode(TOUCH_YM, PinMode_AIn);
	pinMode(TOUCH_YP, PinMode_AIn);
	if (LCD.Orientation == ORIENT_0 || LCD.Orientation == ORIENT_3)
	{
		pinHigh(TOUCH_XP);
		pinLow(TOUCH_XM);
	}
	else
	{
		pinLow(TOUCH_XP);
		pinHigh(TOUCH_XM);
	}
	pinMode(TOUCH_XP, PinMode_Out_PP);
	pinMode(TOUCH_XM, PinMode_Out_PP);
	ADC_RegularChannelConfig(ADC1, TOUCH_YM_CH, 1, TOUCH_SAMPLE);
}

/**********************************************************************
 *
 *
 *********************************************************************/
void DisplayInit()	
{
	const uint8_t * cmds;
	uint8_t idx, data;

	// Port B
	LCD_DATA_IN();

	LCD_BL_LOW();	pinMode(LCD_BACKLIGHT, PinMode_Out_PP_2M);
	LCD_RST_LOW();	pinMode(LCD_RESET,  PinMode_Out_PP_2M);

	LCD_CS_HIGH();	pinMode(LCD_CS,  PinMode_Out_PP);
	LCD_RD_HIGH();	pinMode(LCD_RD,  PinMode_Out_PP);
	LCD_WR_HIGH();	pinMode(LCD_WR,  PinMode_Out_PP);
	LCD_CMD_LOW();	pinMode(LCD_CMD,  PinMode_Out_PP);

	Delay_ms(2);
	LCD_RST_HIGH();
	Delay_ms(5);

	LCD_DATA_OUT();		// PB0-PB15  output

	LCD_CS_LOW();
	cmds = &InitRegs[0];
	for (;;)
	{
		idx = *cmds++;
		data = *cmds++;
		if (idx == 0)
		{	// special command (delay or end)
			if (data == 0)
				break;
			Delay_ms(data);
		}
		else
			LcdWriteReg(idx, data);
	}
	LCD_CS_HIGH();

	LCD.CursorState = 0;
	SetOrientation(ORIENT_0);

	LCD.ColorFG = GREEN224;
	LCD.ColorBG = BLACK;

	ClearScreen();
	LCD_BL_HIGH();

	SelectTouchNone();

	/*
	SelectTouchX();
	TouchState	= TOUCH_XM_START;
	TouchIdx	= 0;
	AdcStartDelay = 2;
	*/

	LCD.FontTable = Font8x18_Consolas;
	// FontTable = Font8x16_System;
	// FontTable = Font8x8_Small;
	// FontTable = Font8x16_Gothic;

	LCD.FontScale = 1;
	LCD.TextCol = 0;
	LCD.TextRow = 0;

	LCD.AutoScroll	= 1;
	LCD.AutoWrap	= 1;
	SetScrollMode(0, 0);
	CursorTimer = CURSOR_DELAY;
}

/**********************************************************************
 *
 *********************************************************************/
void SetAutoWrap(uint8_t wrap)
{
	LCD.AutoWrap = wrap;
}

/**********************************************************************
 *
 *********************************************************************/
void Pixel(int x, int y, COLOR color)
{
	LCD_CS_LOW();

	LcdWriteReg16 (REG_02, x);
	LcdWriteReg16 (REG_06, y);

	LcdWriteCommand22();
	LcdWrite(color);

	LCD_CS_HIGH();
}

/**********************************************************************
 *
 *********************************************************************/
void RectFill(int x, int y, int w, int h, COLOR color)
{
	uint32_t pixel = w * h;

	LCD_CS_LOW();
	LcdSetAddress(x, y, w, h);
	LcdWriteCommand22();
	while(pixel != 0)
	{
		--pixel;
		LcdWrite(color);
	}
	LCD_CS_HIGH();
}

/**********************************************************************
 *
 *********************************************************************/
int PutCharXY(int x, int y, char c)
{
	uint8_t i, j, sy, sx, mask;
	const uint8_t * bytes = LCD.FontTable;
	uint8_t w = *bytes++;
	uint8_t h = *bytes++;
	bytes += (c - ' ') * h;
	
	LCD_CS_LOW();
	LcdSetAddress(x, y, w * LCD.FontScale, h * LCD.FontScale);
	LcdWriteCommand22();

	for (j = 0; j < h; j++)
	{
		c = *bytes++;
		for (sy = 0; sy < LCD.FontScale; sy++)
		{
			mask = 0x80;
			for (i = 0; i < w; i++)
			{
				for (sx = 0; sx < LCD.FontScale; sx++)
				{
					if ((c & mask) == 0)
						LcdWrite(LCD.ColorBG);
					else
						LcdWrite(LCD.ColorFG);
				}
				mask >>= 1;
			}
		}
	}
	LCD_CS_HIGH();
	return (x + w * LCD.FontScale);
}

/**********************************************************************
 *	putc prototype for printf
 *********************************************************************/
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
PUTCHAR_PROTOTYPE
{
	DisplayChar(ch);
	return ch;
}

void CursorOffXY(int x, int y, int w, int h)
{
	RectFill(x, y, w, h, LCD.ColorBG);
	LCD.CursorState = 0;
	CursorTimer = CURSOR_DELAY;
}

/**********************************************************************
 *
 *********************************************************************/
void CursorOff()
{
	const uint8_t * bytes = LCD.FontTable;
	int w = (*bytes++) * LCD.FontScale;
	int h = (*bytes) * LCD.FontScale;
	int x = LCD.TextCol *  w;
	int y = LCD.TextRow * h;
	CursorOffXY(x, y, w, h);
}

/**********************************************************************
 *
 *********************************************************************/
void CursorOn()
{
	const uint8_t * bytes = LCD.FontTable;
	int w = (*bytes++) * LCD.FontScale;
	int h = (*bytes) * LCD.FontScale;
	int x = LCD.TextCol *  w;
	int y = LCD.TextRow * h;

	RectFill(x, y + h - 2, w, 2, LCD.ColorFG);
	LCD.CursorState = 1;
	CursorTimer = CURSOR_DELAY;
}

/**********************************************************************
 *
 *********************************************************************/
void DisplayChar(char c)
{
	const uint8_t * bytes = LCD.FontTable;
	int w = (*bytes++) * LCD.FontScale;
	int h = (*bytes) * LCD.FontScale;
	int x = LCD.TextCol *  w;
	int y = LCD.TextRow * h;

	if (c == '\n' || c == '\r')
	{
		if (LCD.CursorState != 0)
			CursorOffXY(x, y, w, h);
		LcdNextLine();
	}
	else if (c == '\r')
		LCD.TextCol = 0;
	else if (LCD.TextCol < (Width() / w) && c >= ' ' && c <= 0x7E)
	{
		if (LCD.CursorState != 0)
			CursorOffXY(x, y, w, h);

		PutCharXY(x, y, c);
		LCD.TextCol++;
		if (LCD.AutoWrap != 0 && LCD.TextCol >= (Width() / w))
			LcdNextLine();
	}
}

/**********************************************************************
 *
 *********************************************************************/
void DisplayAt(uint8_t column, uint8_t row)
{
	int row_height = LCD.FontTable[1] * LCD.FontScale;

	LCD.TextCol = column;
	LCD.TextRow = row;
	if (LCD.ScrollAddr != 0)
	 	LCD.TextRow = LCD.ScrollAddr / row_height - 1;
	LCD.TextRow %= (Height() / row_height);
}

/**********************************************************************
 *
 *********************************************************************/
void DisplayCharAt(uint8_t column, uint8_t row, char c)
{
	DisplayAt(column, row);
	DisplayChar(c);
}

/**********************************************************************
 *
 *********************************************************************/
void DisplayTextLine(const char * text)
{
	DisplayText(text);
	DisplayChar('\n');
}

/**********************************************************************
 *
 *********************************************************************/
void DisplayText(const char * text)
{
	char c;

	while((c = *text++) != 0)
	{
		if (c == '\n')
			LcdNextLine();
		else if (c == '\r')
			LCD.TextCol = 0;
		else
			DisplayChar(c);
	}
}

/**********************************************************************
 *
 *********************************************************************/
void DisplayTextAt(uint8_t column, uint8_t row, const char * text)
{
	DisplayAt(column, row);
	DisplayText(text);
}

/**********************************************************************
 *
 *********************************************************************/
int DisplayTextXY(int x, int y, const char * text)
{
	char c;
	while((c = *text++) != 0)
	{
		if (c == '\n')
		{
			x = 0;
			y += (LCD.FontTable[1] * LCD.FontScale);
		}
		else if (c == '\r')
		{
			x = 0;
		}
		else
		{
			x = PutCharXY(x, y, c);
		}
	}
	return x;
}

/**********************************************************************
 *	Set background color
 *********************************************************************/
void SetBackground(COLOR color)
{
	LCD.ColorBG = color;
}

/**********************************************************************
 *	Set foreground color
 *********************************************************************/
void SetForeground(COLOR color)
{
	LCD.ColorFG = color;
}

/**********************************************************************
 *	Convert R,G,B to 16 bit color
 *********************************************************************/
COLOR ColorFromRGB(uint8_t R, uint8_t G, uint8_t B)
{
	return (COLOR)(((R & 0x1F) << 11) || ((G & 0x2F) << 5) || ((B & 0x1F) << 0));
}

/**********************************************************************
 *	Set scale for font
 *********************************************************************/
void SetFontScale(uint8_t scale)
{
	LCD.FontScale = scale;
}

/**********************************************************************
 *	Clear screen with background color
 *********************************************************************/
void ClearScreen()
{
	ClearScreenColor(LCD.ColorBG);
}

/**********************************************************************
 *	Clear screen with color
 *********************************************************************/
void ClearScreenColor(COLOR color)
{
	LCD.ScrollAddr = 0;
	LcdSetScroll();
	RectFill(0, 0, Width(), Height(), color);
	DisplayAt(0, 0);
}

/**********************************************************************
 *	Draw circle
 *********************************************************************/
void Circle(int x0, int y0, int r, COLOR color)
{
	int draw_x0, draw_y0;
	int draw_x1, draw_y1;
	int draw_x2, draw_y2;
	int draw_x3, draw_y3;
	int draw_x4, draw_y4;
	int draw_x5, draw_y5;
	int draw_x6, draw_y6;
	int draw_x7, draw_y7;
	int xx, yy;
	int di;
	int w = Width();
	int h = Height();

	if (r == 0)	/* no radius */
		return;

	draw_x0 = draw_x1 = x0;
	draw_y0 = draw_y1 = y0 + r;

	if (draw_y0 < h)
		Pixel(draw_x0, draw_y0, color);	 /* 90 degree */

	draw_x2 = draw_x3 = x0;
	draw_y2 = draw_y3 = y0 - r;
	if (draw_y2 >= 0)
		Pixel(draw_x2, draw_y2, color);	/* 270 degree */

	draw_x4 = draw_x6 = x0 + r;
	draw_y4 = draw_y6 = y0;
	if (draw_x4 < w)
		Pixel(draw_x4, draw_y4, color);	 /* 0 degree */

	draw_x5 = draw_x7 = x0 - r;
	draw_y5 = draw_y7 = y0;
	if (draw_x5>=0)
		Pixel(draw_x5, draw_y5, color);	 /* 180 degree */

	if (r == 1)
		return;

	di = 3 - 2 * r;
	xx = 0;
	yy = r;
	while (xx < yy)
	{
		if (di < 0)
		{
			di += 4 * xx + 6;
		}
		else
		{
			di += 4 * (xx - yy) + 10;
			yy--;
			draw_y0--;
			draw_y1--;
			draw_y2++;
			draw_y3++;
			draw_x4--;
			draw_x5++;
			draw_x6--;
			draw_x7++;
		}
		xx++;
		draw_x0++;
		draw_x1--;
		draw_x2++;
		draw_x3--;
		draw_y4++;
		draw_y5++;
		draw_y6--;
		draw_y7--;

		if (draw_x0 <= w && draw_y0 >= 0)
			Pixel(draw_x0, draw_y0, color);

		if (draw_x1 >= 0 && draw_y1 >= 0)
			Pixel(draw_x1, draw_y1, color);

		if (draw_x2 <= w && draw_y2 <= h)
			Pixel(draw_x2, draw_y2, color);

		if (draw_x3 >=0 && draw_y3 <= h)
			Pixel(draw_x3, draw_y3, color);

		if (draw_x4 <= w && draw_y4 >= 0)
			Pixel(draw_x4, draw_y4, color);

		if (draw_x5 >= 0 && draw_y5 >= 0)
			Pixel(draw_x5, draw_y5, color);

		if (draw_x6 <= w && draw_y6 <= h)
			Pixel(draw_x6, draw_y6, color);

		if (draw_x7 >= 0 && draw_y7 <= h)
			Pixel(draw_x7, draw_y7, color);
	}
}

/**********************************************************************
 *	Draw filled circle
 *********************************************************************/
void CircleFill(int x, int y, int r, COLOR color)
{
	int i;
	for (i = 0; i <= r; i++)
		Circle(x, y, i, color);
}

/**********************************************************************
 *	Draw horizontal line
 *********************************************************************/
void LineH(int x0, int x1, int y, COLOR color)
{
	int w;
	w = x1 - x0 + 1;
	
	LCD_CS_LOW();
	LcdSetAddress(x0, y, w, 1);
	LcdWriteCommand22();
	while (w != 0)
	{
		--w;
		LcdWrite(color);
	}
	LCD_CS_HIGH();
}

/**********************************************************************
 *	Draw vertical line
 *********************************************************************/
void LineV(int x, int y0, int y1, COLOR color)
{
	int h;
	h = y1 - y0 + 1;

	LCD_CS_LOW();
	LcdSetAddress(x, y0, 1, h);
	LcdWriteCommand22();
	while (h != 0)
	{
		--h;
		LcdWrite(color);
	}
	LCD_CS_HIGH();
}

/**********************************************************************
 *	Draw line
 *********************************************************************/
void Line(int x0, int y0, int x1, int y1, COLOR color)
{
	int   dx = 0, dy = 0;
	int   dx_sym = 0, dy_sym = 0;
	int   dx_x2 = 0, dy_x2 = 0;
	int   di = 0;

	dx = x1 - x0;
	if (dx == 0)	/* vertical line */
	{
		if (y1 > y0)	LineV(x0, y0, y1, color);
		else			LineV(x0, y1, y0, color);
		return;
	}

	dy = y1 - y0;
	if (dy == 0)	/* horizontal line */
	{
		if (x1 > x0)	LineH(x0, x1, y0, color);
		else			LineH(x1, x0, y0, color);
		return;
	}

	dx_sym = (dx > 0 ? 1 : -1);
	dy_sym = (dy > 0 ? 1 : -1);
	
	dx = dx_sym * dx;
	dy = dy_sym * dy;

	dx_x2 = dx * 2;
	dy_x2 = dy * 2;

	if (dx >= dy)
	{
		di = dy_x2 - dx;
		while (x0 != x1)
		{
			Pixel(x0, y0, color);
			x0 += dx_sym;
			if (di < 0)
				di += dy_x2;
			else
			{
				di += dy_x2 - dx_x2;
				y0 += dy_sym;
			}
		}
		Pixel(x0, y0, color);
	}
	else
	{
		di = dx_x2 - dy;
		while (y0 != y1)
		{
			Pixel(x0, y0, color);
			y0 += dy_sym;
			if (di < 0)
				di += dx_x2;
			else
			{
				di += dx_x2 - dy_x2;
				x0 += dx_sym;
			}
		}
		Pixel(x0, y0, color);
	}
}


/**********************************************************************
 *	Draw rectangular
 *********************************************************************/
void Rect(int x0, int y0, int x1, int y1, COLOR color)
{
	if (x1 > x0)	LineH(x0, x1, y0, color);
	else			LineH(x1, x0, y0, color);

	if (y1 > y0)	LineV(x0, y0, y1, color);
	else			LineV(x0, y1, y0, color);

	if (x1 > x0)	LineH(x0, x1, y1, color);
	else			LineH(x1, x0, y1, color);

	if (y1 > y0)	LineV(x1, y0, y1, color);
	else			LineV(x1, y1, y0, color);

	return;
}

/**********************************************************************
 *	Block copy
 *********************************************************************/
/*
uint16_t copy_block[320];	// Maximum width
void XCopy(int x_src, int y_src, int w, int h, int x_dst, int y_dst)
{
	uint16_t *data, i, dx;
	uint16_t r,g,b;

	LCD_CS_LOW();
	while (h != 0)
	{
		data = &copy_block[0];
		LcdSetAddress(x_src, y_src, w, 1);
		LcdWriteCommand22();
		for (i = 0; i < w; i++)
		{
			dx = LcdRead();  // Dummy Read for PMP read acces
			dx = LcdRead();  // Dummy Read for TFT GRAM read acces
			dx = LcdRead();
			r = (dx & 0xFF) >> 10;
			g = (dx & 0xFF) >> 2;
			dx = LcdRead();
			b = (dx & 0xFF00) >> 10;
			dx = 0;

			//			return  (WORD)RGB565CONVERT(r,g,b);
//			dx = (Read() & 0x00FC) << 3;	// G
//			dx |= (Read() & 0x00F8) << 8;	// R
//			dx |= (Read() & 0x00F8) >> 3;	// B
			
			*data++ = dx;
		}

		data = &copy_block[0];
		LcdSetAddress(x_dst, y_dst, w, 1);
		LcdWriteCommand22();
		for (i = 0; i < w; i++)
			LcdWrite(*data++);

		y_src++;
		y_dst++;
		--h;
	}
	LCD_CS_HIGH();
}
*/

/**********************************************************************
 *	Idle hook for display
 *********************************************************************/
void DisplayIdleApps()
{
	if (CursorTimer == 0)
	{
		if (LCD.CursorState != 0)
			CursorOff();
		else
			CursorOn();
	}

#ifdef USE_TOUCH
	if (LCD.TouchState == TOUCH_END)
	{
		uint8_t	row, col;
		uint16_t sz;

		col = LCD.TextCol;
		row = LCD.TextRow;

		sz = sizeof(LCD.TouchXMA) / sizeof(uint16_t);
		
		LCD.TouchXMA[LCD.TouchIdx] = LCD.TouchXM / sz;
		LCD.TouchXPA[LCD.TouchIdx] = LCD.TouchXP / sz;
		LCD.TouchYMA[LCD.TouchIdx] = LCD.TouchYM / sz;
		LCD.TouchYPA[LCD.TouchIdx] = LCD.TouchYP / sz;

		LCD.TouchIdx++;
		if (LCD.TouchIdx == sz)
			LCD.TouchIdx = 0;

		LCD.TouchXM = 0;
		LCD.TouchXP = 0;
		LCD.TouchYM = 0;
		LCD.TouchYP = 0;
		while (sz != 0)
		{
			--sz;
			LCD.TouchXM += LCD.TouchXMA[sz];
			LCD.TouchXP += LCD.TouchXPA[sz];
			LCD.TouchYM += LCD.TouchYMA[sz];
			LCD.TouchYP += LCD.TouchYPA[sz];
		}

		DisplayAt(15, 0);
		printf("XM:%d\n", LCD.TouchXM);
		printf("XP:%d\n", LCD.TouchXP);
		printf("YM:%d\n", LCD.TouchYM);
		printf("YP:%d\n", LCD.TouchYP);
		printf("Z1:%d\n", LCD.TouchZ1);
		printf("Z2:%d\n", LCD.TouchZ2);

		sz = LCD.TouchXM + LCD.TouchXP;

		if (sz != 0)
		{
			printf("XMN:%d\n", LCD.TouchXM * 100 / sz);
			printf("XPN:%d\n", LCD.TouchXP * 100 / sz);
		}

		LCD.TouchState = TOUCH_XM_START;
		SelectTouchX();
		AdcStartDelay = 2;

		LCD.TextCol = col;
		LCD.TextRow = row;
	}
#endif
}

/**********************************************************************
 *
 *
 *********************************************************************/

#ifdef USE_TOUCH
#if defined (STM32F10X_LD_VL) || defined (STM32F10X_MD_VL) || defined (STM32F10X_HD_VL)
void ADC1_IRQHandler(void)
#else
void ADC1_2_IRQHandler(void)
#endif
{
	// uint16_t data = ADC_GetConversionValue(ADC1);
	uint16_t data = ADC_GetConversionValue(ADC1) >> 3;

	switch (LCD.TouchState)
	{
	case TOUCH_XM_START:
		LCD.TouchXM = data;
		ADC_RegularChannelConfig(ADC1, TOUCH_XP_CH, 1, TOUCH_SAMPLE);
		LCD.TouchState++;
		AdcStartDelay = 2;
		break;

	case TOUCH_XP_START:
		LCD.TouchXP = data;
		SelectTouchY();
		LCD.TouchState++;
		AdcStartDelay = 2;
		break;

	case TOUCH_YM_START:
		LCD.TouchYM = data;
		ADC_RegularChannelConfig(ADC1, TOUCH_YP_CH, 1, TOUCH_SAMPLE);
		LCD.TouchState++;
		AdcStartDelay = 2;
		break;

	case TOUCH_YP_START:
		LCD.TouchYP = data;
		SelectTouchPress();
		LCD.TouchState++;
		AdcStartDelay = 2;
		break;

	case TOUCH_PRESS_1:
		LCD.TouchZ1 = data;
		ADC_RegularChannelConfig(ADC1, TOUCH_YP_CH, 1, TOUCH_SAMPLE);
		LCD.TouchState++;
		AdcStartDelay = 2;
		break;

	case TOUCH_PRESS_2:
		LCD.TouchZ2 = data;
		SelectTouchNone();
		LCD.TouchState++;
		break;
	}
}
#endif
