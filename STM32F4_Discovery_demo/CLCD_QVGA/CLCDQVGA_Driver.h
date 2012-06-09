#ifndef COLORLCD_H_
#define COLORLCD_H_

// ChibiOS Core Includes
#include "ch.h"
#include "hal.h"

// Other Includes
#include "fonts.h"

#include <stdio.h>


enum lcd_bl_states {
	LCD_BL_ON,
	LCD_BL_DIM0,
	LCD_BL_DIM,
	LCD_BL_OFF,
	LCD_BL_ONN
};


/* Hardware Defines */

/*
 * QVGA LCD Hardware Pin Assignments
 *
 * LCD_BL			PB8		=	TIM4:CH3
 *
 * LCD_CS			PD7 *
 * LCD_RST			PD10
 * LCD_RS			PD11 *
 *
 * LCD_WR			PD8
 * LCD_RD			PD9
 *
 * LCD_DB[0:3]		PD[0:3]
 * LCD_DB[4:15]		PE[4:15]
 *
 */

#define LCD_BL_GPIO			GPIOB
#define LCD_BL_PIN			8

#define LCD_CS_GPIO			GPIOD
#define LCD_CS_PIN			7

#define LCD_RS_GPIO			GPIOD
#define LCD_RS_PIN			11

#define LCD_RST_GPIO		GPIOD
#define LCD_RST_PIN			10

#define LCD_RD_GPIO			GPIOD
#define LCD_RD_PIN			9

#define LCD_WR_GPIO			GPIOD
#define LCD_WR_PIN			8

#define LCD_D0_GPIO			GPIOD
#define LCD_D4_GPIO			GPIOE


// Colors (8-bit) RRR GGG BB -> RRRG GGBB
#if 1
#define BLACK       0x0000			// 00000 000000 00000
#define RED         0xF800			// 11111 000000 00000
#define GREEN       0x07E0			// 00000 111111 00000
#define BLUE        0x001F			// 00000 000000 11111
#define YELLOW      RED|GREEN
#define CYAN        GREEN|BLUE
#define MAGENTA     RED|BLUE
#define WHITE       RED|BLUE|GREEN

#define PURPLE 	0XE3

#define GRAY1 0b00100100			// darkest
#define GRAY2 0b01001001
#define GRAY3 0b01101101
#define GRAY4 0b10010010
#define GRAY5 0b10110110
#define GRAY6 0b11011011			// lightest

#define BLUE1 0b0000000000000111	// darkest
#define BLUE2 0b0000000000001111
#define BLUE3 0b00000011
#define BLUE4 0b0001100011111111
#define BLUE5 0b0011100111111111
#define BLUE6 0b0111101111111111
#define BLUE7 0b10010011
#define BLUE8 0b10110111
#define BLUE9 0b11011011			// lightest

#define GREEN1 0b00001000			// darkest
#define GREEN2 0b00001100
#define GREEN3 0b00010000
#define GREEN4 0b00110100
#define GREEN5 0b01011000
#define GREEN6 0b01111100
#define GREEN7 0b01011101
#define GREEN8 0b10011110
#define GREEN9 0b11011111			// lightest
#endif

extern const uint16_t colors[];

extern volatile uint16_t cx,cy;
extern volatile uint16_t px,py;
extern volatile uint16_t bgcolor,fgcolor;

extern volatile uint8_t lcd_blstate;
extern volatile uint16_t lcd_bltime;

extern const uint8_t* font;

#define lcd_initc()			{ lcd_init(); lcd_cls();				}
#define lcd_setpixel(p)     { lcd_data(p);                          }
#define lcd_locate(x,y)		{ lcd_writereg(0x20, x); lcd_writereg(0x21, y);	}
#define lcd_gotoxy(x,y)		{ cx=x; cy=y; lcd_locate(x,y);			}

// Send a command to LCD
extern void lcd_cmd(uint16_t c);

// Send data to LCD
extern void lcd_data(uint16_t d);

// Send data to LCD (fast)
extern void lcd_lld_write(uint16_t db);

// Write to LCD Register
extern void lcd_writereg(uint16_t c, uint16_t d);

// Initialize the Color LCD
extern void lcd_init(void);

// Support for LCD Backlight auto-off timer - ChibiOS Integrated
#define LCD_BACKLIGHT_ON_TIME	150
#define LCD_BACKLIGHT_DIM_TIME	600

extern void lcd_blhook(void* p);
extern void lcd_blup(void);

// Clear the screen
extern void lcd_cls(void);

// Set drawing boundary
extern void lcd_setrect(uint16_t xs, uint16_t xe, uint16_t ys, uint16_t ye);

// Draw a filled rectangle with a fixed color
extern void lcd_filledrect(uint16_t clr, uint16_t xs, uint16_t xe, uint16_t w, uint16_t h);

// Switch to the next line
extern void lcd_newline(void);

// Draws an ASCII Character on the screen (Compatible with GNU FILE structures)
extern void lcd_putchar(char c);

// Puts a string on the LCD
extern void lcd_puts(char* c);

// Touch Screen Function
#define tsc_isPenDown()		(!palReadPad(GPIOB, 0))
extern int lcd_getpointraw(uint16_t*, uint16_t*);
extern void tsc_init(void);
extern uint16_t tsc_read(uint8_t addr);

// Point in px, py
int lcd_getpoint(void);


#endif /* COLORLCD_H_ */
