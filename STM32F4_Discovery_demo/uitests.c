/*
 * uitests.c
 *
 *  Created on: Jun 2, 2012
 *      Author: Kumar Abhishek
 */

// This file is currently not used.

#include "ch.h"
#include "hal.h"

#include "lis302dl.h"
#include "chprintf.h"

#include "ff.h"

#include "CLCDQVGA_Driver.h"

#include "UI.h"
#include "keys.h"
#include "xprintf.h"


static void tsc_getCalibPoint(uint16_t vx, uint16_t vy, uint16_t* x, uint16_t* y)
{
	uint16_t _x=0,_y=0;
	lcd_filledrect(BLUE, vx-1, vy-1, 3, 3);
	lcd_filledrect(BLUE, vx-10, vy, 20, 1);
	lcd_filledrect(BLUE, vx, vy-10, 1, 20);

	while (!tsc_isPenDown()) {}

	for (int i=0;i<10;i++)
	{
		uint16_t tx=0,ty=0;
		lcd_getpointraw(&tx, &ty);
		_x+=tx;
		_y+=ty;
	}

	_x/=10;
	_y/=10;

	lcd_filledrect(WHITE, vx-1, vy-1, 3, 3);
	lcd_filledrect(WHITE, vx-10, vy, 20, 1);
	lcd_filledrect(WHITE, vx, vy-10, 1, 20);

	*x=_x;
	*y=_y;
}

void translate(uint16_t* x, uint16_t* y)
{
	int16_t a=*x,b=*y;

	a=(int16_t)(_ax*a+_bx*b+_c);
	b=(int16_t)(_ay*a+_by*b+_d);

	*x=a;
	*y=b;
}

//void translatePrim(uint16_t* x, uint16_t* y)
//{
//
//}

// Three point calibration
void tsc_calibration(void)
{
	uint16_t x1,y1,x2,y2,x3,y3;

	uint16_t X1=100, Y1=200,
			 X2=180, Y2=160,
			 X3=200, Y3=200;

	double dx, dx1, dx2, dx3, dy1, dy2, dy3;

	cx=3; cy=3; 	lcd_puts("Calibration");
	cx=3; cy=20;	lcd_puts("Tap the red point.");

	chprintf(&SD2, "Touch Panel Calibration Started\r\n");

	x1=x2=x3=y1=y2=y3=0;

	tsc_getCalibPoint(X1, Y1, &x1, &y1);
	chprintf(&SD2, "X1=%d, Y1=%d\r\n", x1, y1);

	chThdSleepMilliseconds(1000);

	tsc_getCalibPoint(X2, Y2, &x2, &y2);
	chprintf(&SD2, "X2=%d, Y2=%d\r\n", x2, y2);

	chThdSleepMilliseconds(1000);

	tsc_getCalibPoint(X3, Y3, &x3, &y3);
	chprintf(&SD2, "X3=%d, Y3=%d\r\n", x3, y3);

	chThdSleepMilliseconds(1000);

	dx =((double)(x1-x3)) * ((double)(y2-y3)) - ((double)(x2-x3)) * ((double)(y1-y3));
	dx1=((double)(X1-X3)) * ((double)(y2-y3)) - ((double)(X2-X3)) * ((double)(y1-y3));
	dx2=((double)(x1-x3)) * ((double)(x2-x3)) - ((double)(x2-x3)) * ((double)(X1-X3));
	dx3=X1*((double)x2*(double)y3 - (double)x3*(double)y2) -
			X2*((double)x1*(double)y3 - (double)x3*(double)y1) +
			X3*((double)x1*(double)y2 - (double)x2*(double)y1);

	dy1=((double)(Y1-Y3)) * ((double)(y2-y3)) - ((double)(Y2-Y3)) * ((double)(y1-y3));
	dy2=((double)(x1-x3)) * ((double)(Y2-Y3)) - ((double)(x2-x3)) * ((double)(Y1-Y3));
	dy3=Y1*((double)x2*(double)y3 - (double)x3*(double)y2) -
			Y2*((double)x1*(double)y3 - (double)x3*(double)y1) +
			Y3*((double)x1*(double)y2 - (double)x2*(double)y1);

//	chprintf(&SD2, "dx=%ld, dx1=%ld, dx2=%ld, dx3=%ld\r\n", dx, dx1, dx2, dx3);
//	chprintf(&SD2, "dy1=%ld, dy2=%ld, dy3=%ld\r\n", dy1, dy2, dy3);

	_ax=(double)dx1/(double)dx;
	_bx=(double)dx2/(double)dx;
	_c=(double)dx3/(double)dx;

	_ay=(double)dy1/(double)dx;
	_by=(double)dy2/(double)dx;
	_d=(double)dy3/(double)dx;

	translate(&x1, &y1);
	translate(&x2, &y2);
	translate(&x3, &y3);
	chprintf(&SD2, "X1=%d, Y1=%d\r\n", x1, y1);
	chprintf(&SD2, "X2=%d, Y2=%d\r\n", x2, y2);
	chprintf(&SD2, "X3=%d, Y3=%d\r\n", x3, y3);
}

