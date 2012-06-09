/*
 * buttons.h
 *
 *  Created on: Jun 15, 2010
 *      Author: Kumar Abhishek
 */

#include <stdint.h>

#ifndef BUTTONS_H_
#define BUTTONS_H_

enum BUTTONS {
	BTN_NONE,
	BTN_MID_UP,
	BTN_MID_DOWN,
	BTN_MID_LEFT,
	BTN_MID_RIGHT,
	BTN_LEFT,
	BTN_RIGHT,
	BTN_MID_SEL,
	BTN_HOLD_1,
	BTN_HOLD_2
};

extern uint8_t key;
extern volatile uint16_t keytime;

extern void getKeypress(void);
extern uint8_t readKeyPort(void);


#endif /* BUTTONS_H_ */
