// File adopted from ChibiOS STM32F4Discovery Demo

#include "ch.h"
#include "hal.h"

#include "chprintf.h"

// User Includes
#include "codec.h"

Thread* playerThread;

// Application Title
const char appTitle[]="STM32F4D-ChibiOS Audio Demo";
Thread* mainThread;

/*
 * This is a periodic thread that does absolutely nothing except flashing
 * a LED.
 */
static WORKING_AREA(waThread1, 128);
__attribute__  ((noreturn))
static msg_t Thread1(void *arg) {
	(void)arg;
	chRegSetThreadName("blinker");

	codec_i2s_init(44100, 16);

	codec_pwrCtl(1);    // POWER ON
	codec_muteCtl(0);   // MUTE OFF
	while (TRUE)
	{
		palSetPad(GPIOD, GPIOD_LED3);       /* Orange.  */
		chThdSleepMilliseconds(500);
		palClearPad(GPIOD, GPIOD_LED3);     /* Orange.  */
		chThdSleepMilliseconds(500);
		codec_sendBeep();
	}
}

// Hardware Initialization
static void hw_init(void)
{
	// Init Serial Port
	sdStart(&SD2, NULL);
	palSetPadMode(GPIOA, 2, PAL_MODE_ALTERNATE(7));
	palSetPadMode(GPIOA, 3, PAL_MODE_ALTERNATE(7));

	codec_hw_init();
}

static VirtualTimer vt1;

static void led_toggle(void *p)
{
	(void)p;
	palTogglePad(GPIOD, GPIOD_LED4);
	chVTSetI(&vt1, 500, led_toggle, NULL);
}

// Entry Point
int main(void)
{
	halInit();
	chSysInit();

	mainThread=chThdSelf();

	hw_init();

	//chThdCreateFromHeap(NULL, 1024, NORMALPRIO, Thread1, NULL);
	playerThread=chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);

	chVTSetI(&vt1, 500, led_toggle, NULL);

	while (TRUE) {
		chThdSleep(100);
	}
}
