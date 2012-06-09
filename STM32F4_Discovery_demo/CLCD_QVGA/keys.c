/*
 * keys.c
 *
 *  Created on: Jun 15, 2010
 *      Author: Kumar Abhishek
 */

#include "keys.h"

uint8_t key;
//extern volatile uint16_t keytime;

//extern void lcd_blup(void);

#define MAX7329_addr	0x70

#ifdef IO_EXPANDERS

uint8_t readKeyPort()
{
	i2c_start(MAX7329_addr+1);
	uint8_t retval=i2c_readAck();
	i2c_readNak();
	return retval;
}

#else

uint8_t readKeyPort()
{
//	asm volatile ("nop");
//	return (PINC&0xFC)>>2;
	return 0;
}

#endif

#ifdef IO_EXPANDERS
void getKeypress()
{
	key=BTN_NONE;

	static uint8_t keys=0;
	uint8_t keys2=(~readKeyPort())&0x7F;

	if (keys)
	{
		if (keys2==0) {
			if (keytime > 8) {
				switch (keys)
				{
					case 0x01:	key=BTN_MID_UP;
								break;

					case 0x02:	key=BTN_MID_DOWN;
								break;

					case 0x04:	key=BTN_MID_LEFT;
								break;

					case 0x08:	key=BTN_MID_RIGHT;
								break;

					case 0x10:	key=BTN_MID_SEL;
								break;

					case 0x20:	key=BTN_RIGHT;
								break;
				}
				keys=0;
			}
		}
	}
	else
	{
		keytime=0;
		keys=(~readKeyPort())&0x7F;
	}
	if (key!=0) lcd_blup();
}
#else
void getKeypress()
{
//	key=BTN_NONE;
//
//	static uint8_t keyState=0;
//	static uint8_t keyThres=8;
//
//	DDRC&=0x03; PORTC|=0xFC;
//
//	asm volatile ("nop");
//	asm volatile ("nop");
//
//	static uint8_t keys=0;
//
//	uint8_t t=0;
//
//	static uint8_t key_tmp=0;
//
//	switch (keyState)
//	{
//		case 0: keys=((~PINC&0xFC)>>2);
//				if (keys) keyState=1;
//				keyThres=8;
//				keytime=0;
//				break;
//
//		case 1:	t=((~PINC&0xFC)>>2);
//				if (t==0 || keys!=t ) { keyState=0; break; }
//				if (keytime > keyThres && keytime < keyThres+3) {
//					switch (keys)
//					{
//						case 0x01:	key_tmp=BTN_MID_UP;
//									break;
//
//						case 0x02:	key_tmp=BTN_MID_DOWN;
//									break;
//
//						case 0x04:	key_tmp=BTN_MID_LEFT;
//									break;
//
//						case 0x08:	key_tmp=BTN_MID_RIGHT;
//									break;
//
//						case 0x10:	key_tmp=BTN_MID_SEL;
//									break;
//
//						case 0x20:	key_tmp=BTN_RIGHT;
//									break;
//					}
//					key=key_tmp;
//					keyState=2;
//				}
//				break;
//
//		case 2:	t=((~PINC&0xFC)>>2);
//				if (t==0 || keys!=t ) { keyState=0; break; }
//
//				if (key_tmp && key_tmp <= BTN_MID_RIGHT)
//				{
//					keytime=0;
//					keyThres=20;
//					keyState=1;
//				}
//
//				if (key_tmp==BTN_RIGHT && keytime > 200)
//				{
//					key=BTN_HOLD_1;
//					keyState=3;
//				}
//				break;
//
//		case 3: t=((~PINC&0xFC)>>2);
//				if (t==0 || keys!=t ) { keyState=0; break; }
//	}
//
//	if (key!=0) lcd_blup();
	key=0;
}

#endif
