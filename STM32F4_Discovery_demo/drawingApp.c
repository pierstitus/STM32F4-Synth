/*
 * drawingApp.c
 *
 *  Created on: Jun 5, 2012
 *      Author: Kumar Abhishek
 */

#include "drawingApp.h"

static WORKING_AREA(waThread2, 1024);
static Thread* drawThd;

static msg_t DrawingThread(void *arg);

// Blocking method
// Will return after thread exits
void startDrawingThread(void)
{
	ui_clearUserArea();

	ui_drawTitleBar("Drawing Application");
	ui_drawBottomBar(NULL, "Touch To Exit", NULL);

	drawThd=chThdCreateStatic(waThread2, sizeof(waThread2), NORMALPRIO-1, DrawingThread, NULL);
	chThdWait(drawThd);

	lcd_cls();
	ui_displayPreviousMenu();
}

// Drawing Thread

#define NUM_CONVERSIONS	4

static msg_t DrawingThread(void *arg)
{
	(void)arg;
	chRegSetThreadName("paint");

	int state=0, i, bg, fg, dx=0, dy=0;

	bg=7;
	fg=0;

	for (i=0;i<7;i++)
	{
		lcd_filledrect(colors[i], 3+i*22, UI_TBARHEIGHT+3, 20, 20);
	}

	i=0;
	while (TRUE)
	{
		if (state)
		{
			if (!tsc_isPenDown())
			{
				state=0;
				// Allow time to settle
				chThdSleepMilliseconds(60);
			}
			else if (lcd_getpoint())
			{
				// Keep the lcd backlight on
				lcd_blup();

				// Check if color toolbar is hit
				if (py > UI_TBARHEIGHT+2 && py < 50)
				{
					int t=(px-4)/22;
					if (t<7)
					{
						lcd_filledrect(colors[fg], 10+fg*22, UI_TBARHEIGHT+10, 4, 4);
						fg=t;
						lcd_filledrect(fg==0 ? WHITE : BLACK, 10+fg*22, UI_TBARHEIGHT+10, 4, 4);
					}

					// Fill background with selected color
					else if (t==7)
					{
						bg=fg;
						lcd_filledrect(colors[bg], 0, UI_TBARHEIGHT+3+20+1, 240, 320-(UI_TBARHEIGHT+3+20+3+UI_BBARHEIGHT));
					}
				}

				// If within boundary, plot the point
				if (py > 50 && py < 285 && px > 2 && px < 315) {
					if (i==NUM_CONVERSIONS) {
						lcd_filledrect(colors[fg], dx/NUM_CONVERSIONS, dy/NUM_CONVERSIONS, 2, 2);
						i=dx=dy=0;
					} else {
						dx+=px;
						dy+=py;
						i++;
					}
				}

				// If exit button clicked
				if (py > 290 && py < 320 && px > 200 && px < 240)
					return 0;
			}
		}
		else if (tsc_isPenDown())
		{
			state=1;
			// Allow time to settle
			chThdSleepMilliseconds(60);
		}
	}

	return 0;
}
