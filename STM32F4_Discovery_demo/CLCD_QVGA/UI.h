/*
 * UI.h
 *
 *  Created on: Jun 14, 2010
 *      Author: Kumar Abhishek
 *
 *	Initial: Code Created for my AVR ATMega32 platform
 *
 *	September 2011: Ported to the STM32 Platform
 *
 *	1 June 2012: Changes made for it to run smoothly with my QVGA Display
 *
 *  8 June 2012: Major revisions, tidying the code
 *
 *  The UI code as a whole, is based on the UI framework provided on
 *  this website: http://reifel.org/PICUserInterface/ . It was actually the
 *  first piece of code from where I learnt UI fundamentals, like event handler
 *  mechanism.
 *
 */

#ifndef UI_H_
#define UI_H_

#include "CLCDQVGA_Driver.h"
#include "fonts.h"
#include "keys.h"

#define UI_DISPLAY_WIDTH		240
#define UI_DISPLAY_HEIGHT		320

#define UI_TBARSTARTX			0
#define UI_TBARSTARTY			0
#define UI_TBARWIDTH			UI_DISPLAY_WIDTH
#define UI_TBARHEIGHT			25
#define UI_TBARCOLOR			BLUE1

#define UI_BBARSTARTX			0
#define UI_BBARSTARTY			(320-UI_BBARHEIGHT)
#define UI_BBARWIDTH			UI_DISPLAY_WIDTH
#define UI_BBARHEIGHT			20
#define UI_BBARCOLOR			BLUE6
#define UI_BBARCOLOR2			BLUE5

#define UI_MENUSELCOLOR			BLUE4
#define UI_MSELGRD0				BLUE2
#define UI_MSELGRD1				BLUE5
#define UI_MSELGRD2				BLUE2

#define UI_BARFONT				font_Larger
#define UI_FONT					font_MediumBold

enum menu_types {
	MENU_HEADER,
	MENU_SUBHEADER,
	MENU_ITEM,
	MENU_SUBMENU,
	MENU_END
};

typedef struct _menuItem{
	uint8_t type;
	const char* name;
	uint8_t* tag;
} menuItem;

enum stringTypes {
	TEXT_ALIGN_LEFT,
	TEXT_ALIGN_CENTER,
	TEXT_ALIGN_RIGHT
};

#define pfn				void (*)(void)
#define pfn_T(n)		void (*n)()

extern const char appTitle[];

extern FILE lcd_stream;

#define ui_getCurFontHeight()		(font[FONT_TABLE_HEIGHT_IDX])
extern uint8_t ui_getCharWidth(uint8_t c);
extern uint16_t ui_getWordWidth(const char* c);

extern void ui_drawString(const char* t, uint8_t align);
extern void ui_drawTitleBar(const char* t);
extern void ui_drawBottomBar(const char* l, const char* c, const char* r);

extern void ui_drawMenuItem(uint8_t ix);
extern void ui_displayMenu(const menuItem* it, uint8_t sel);
extern void ui_displayPreviousMenu(void);

extern void ui_drawSlide(uint8_t clr, uint8_t max, uint8_t set);

extern void ui_sethandler(pfn_T(hFunc));
extern void ui_executecmd(uint8_t cmd);

extern void ui_clearUserArea(void);

#endif /* UI_H_ */
