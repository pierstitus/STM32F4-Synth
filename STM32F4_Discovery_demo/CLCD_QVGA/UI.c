/*
 * UI.c
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

#include "UI.h"
#include "keys.h"

pfn_T(curHandler);

menuItem* currentMenu;

static uint8_t curs;
static uint8_t prevs;

uint8_t ui_getCharWidth(uint8_t c)
{
	const uint8_t* ptr;

	uint8_t sps=font[FONT_TABLE_PAD_AFTER_CHAR_IDX];

	uint16_t chi;

	if (c<0x20||c>0x7F) {
		return 0;
	}

	chi=*(uint16_t*)(&font[FONT_TABLE_CHAR_LOOKUP_IDX+ ((c-0x20)<<1)]);

	ptr=font+chi;

	uint8_t fontWidth=*(ptr++);

	return fontWidth+sps;
}

uint16_t ui_getWordWidth(const char* c)
{
	uint16_t result=0;
	char d;

	while ((d=*c++))
		result+=ui_getCharWidth(d);

	return result;
}

void ui_drawString2(const char* t, uint8_t align)
{
	switch(align)
	{
		case TEXT_ALIGN_LEFT:	cx=1;
								break;

		case TEXT_ALIGN_CENTER:	cx=((UI_DISPLAY_WIDTH-ui_getWordWidth(t))/2)+1;
								break;

		case TEXT_ALIGN_RIGHT:	cx=(UI_DISPLAY_WIDTH-ui_getWordWidth(t));
								break;
	}

	lcd_puts((char*)t);
}

void ui_drawString(const char* t, uint8_t align)
{
	uint16_t _cx=cx;
	ui_drawString2(t,align);
	cx=_cx;
}

void ui_drawTitleBar(const char* t)
{
	uint16_t _cx=bgcolor,_cy=fgcolor;

	bgcolor=UI_TBARCOLOR; fgcolor=WHITE;

	font=font_Larger;

	lcd_filledrect(bgcolor, UI_TBARSTARTX, UI_TBARSTARTY,
				   UI_TBARWIDTH, UI_TBARHEIGHT);

	cy=(UI_TBARHEIGHT-ui_getCurFontHeight())/2 + 1;
	ui_drawString(t, TEXT_ALIGN_CENTER);

	font=font_MediumBold;

	bgcolor=_cx;
	fgcolor=_cy;
}

void ui_drawBottomBar(const char* l, const char* c, const char* r)
{
	uint16_t _cx=bgcolor,_cy=fgcolor;

	bgcolor=UI_BBARCOLOR; fgcolor=WHITE;

	font=font_Larger;

	lcd_filledrect(bgcolor, UI_BBARSTARTX, UI_BBARSTARTY+2,
					   UI_BBARWIDTH, UI_BBARHEIGHT);

	lcd_filledrect(UI_BBARCOLOR2, UI_BBARSTARTX, UI_BBARSTARTY, UI_BBARWIDTH, 2);

	font=font_Larger;

	cy=UI_BBARSTARTY+(UI_BBARHEIGHT-ui_getCurFontHeight())/2 + 1;

	if (c) ui_drawString(c, TEXT_ALIGN_CENTER);

	cy+=3;

	font=font_Small;
	if (l) ui_drawString(l, TEXT_ALIGN_LEFT);
	if (r) ui_drawString(r, TEXT_ALIGN_RIGHT);

	font=font_MediumBold;

	bgcolor=_cx;
	fgcolor=_cy;
}

void ui_getMenuItem(menuItem* it, uint8_t i)
{
	it->name=currentMenu[i].name;
	it->tag=currentMenu[i].tag;
	it->type=currentMenu[i].type;
}

void ui_drawMenuItem(uint8_t ix)
{
	uint16_t _f=fgcolor, _b=bgcolor;
	menuItem curItem;

	ui_getMenuItem(&curItem, ix);

	font=font_MediumBold;

	cx=1;
	cy=UI_TBARHEIGHT+(ix*20)-18;

	if (ix!=curs)
	{
		fgcolor=BLACK;
		bgcolor=WHITE;

		lcd_filledrect(bgcolor, cx, cy, UI_DISPLAY_WIDTH-2, 3);
		cy+=3;
	}
	else
	{
		bgcolor=UI_MENUSELCOLOR;
		fgcolor=WHITE;

		lcd_filledrect(UI_MSELGRD0, cx, cy, UI_DISPLAY_WIDTH-2, 1); cy++;
		lcd_filledrect(UI_MSELGRD1, cx, cy, UI_DISPLAY_WIDTH-2, 1); cy++;
		lcd_filledrect(bgcolor, cx, cy, UI_DISPLAY_WIDTH-2, 1); cy++;
	}

	lcd_filledrect(bgcolor,cx,cy,UI_DISPLAY_WIDTH-2,ui_getCurFontHeight());
	ui_drawString2(curItem.name, TEXT_ALIGN_LEFT);

	cx=1;

	cy+=ui_getCurFontHeight();

	if (ix!=curs) {
		lcd_filledrect(WHITE, cx, cy, UI_BBARWIDTH-2, 2);
		cy+=2;
	} else {
		lcd_filledrect(UI_MSELGRD1, cx, cy, UI_DISPLAY_WIDTH-2, 1); cy++;
		lcd_filledrect(UI_MSELGRD2, cx, cy, UI_DISPLAY_WIDTH-2, 1); cy++;
	}
	fgcolor=_f; bgcolor=_b;
}

static void menuUpdateSelection(uint8_t new)
{
	menuItem sel;
	uint8_t prevSel;

	ui_getMenuItem(&sel, new);

	if ((new >= 1) &&
		(sel.type != MENU_END) &&
	    (new !=curs))
	{	//
		// select the new menu item
		//
		prevSel = curs;
		curs = new;

		ui_drawMenuItem(curs);
		ui_drawMenuItem(prevSel);
	}
}

static void menuDisplayHandler(uint8_t cmd)
{
	menuItem cur;
	ui_getMenuItem(&cur, curs);

	switch(cmd)
	{
		case BTN_MID_DOWN:
			menuUpdateSelection(curs+1);
			break;

		case BTN_MID_UP:
			menuUpdateSelection(curs-1);
			break;

		case BTN_RIGHT:
			ui_getMenuItem(&cur, 0);
			if (cur.tag && cur.type==MENU_HEADER) ((pfn)(cur.tag))();
			else if (cur.tag && cur.type==MENU_SUBHEADER) {
				ui_clearUserArea();
				ui_displayMenu((menuItem*)cur.tag,prevs);
				prevs=1;
			}
			break;

		case BTN_MID_SEL:
			switch(cur.type)
			{
				case MENU_ITEM:		if (cur.tag) ((pfn)(cur.tag))();
									break;

				case MENU_SUBMENU: 	ui_clearUserArea();
									prevs=curs;
									ui_displayMenu((menuItem*)cur.tag,1);
									break;
			}
			break;
	}
}

void ui_displayMenu(const menuItem* it, uint8_t sel)
{
	currentMenu=(menuItem*)it;
	curs=sel;

	menuItem cur;

	uint8_t i=0;
	do
	{
		ui_getMenuItem(&cur, i);

		if (cur.type==MENU_HEADER || cur.type==MENU_SUBHEADER)
			ui_drawTitleBar(it[i].name);
		else if (cur.type!=MENU_END)
			ui_drawMenuItem(i);

		i++;
	} while (cur.type != MENU_END);

	ui_drawBottomBar(0, (const char*)"Select", (const char*)"Exit");

	ui_sethandler((pfn)menuDisplayHandler);
}

void ui_displayPreviousMenu()
{
	if (currentMenu) ui_displayMenu(currentMenu, curs);
}

void ui_drawSlide(uint8_t clr, uint8_t max, uint8_t set)
{
	int t=120*set/max;

	lcd_filledrect(clr, 3, cy, 123, 1);
	lcd_filledrect(clr, 3, cy, 1, 20);
	lcd_filledrect(clr, 3, cy+19, 123, 1);
	lcd_filledrect(clr, 126, cy, 1, 20);
	lcd_filledrect(bgcolor, 4, cy+1, 1, 18);

	uint8_t i=(uint8_t) t;

	lcd_filledrect(clr, 5, cy+2, i, 16);
	lcd_filledrect(bgcolor, 6+i, cy+2, 120 - i, 16);
}

void ui_sethandler(pfn_T(hFunc))
{
	curHandler=hFunc;
}

void ui_executecmd(uint8_t cmd)
{
	if (curHandler) curHandler(cmd);
}

void ui_clearUserArea()
{
	lcd_filledrect(WHITE,0,UI_TBARHEIGHT,240, 320-(UI_TBARHEIGHT+UI_BBARHEIGHT));
}
