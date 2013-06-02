// File adopted from ChibiOS STM32F4Discovery Demo

#include "ch.h"
#include "hal.h"

#include "chprintf.h"
#include "ff.h"

// User Includes
#include "wavePlayer.h"

void mainEventHandler(uint8_t evt);

void playWaveFile(void);

// Application Title
const char appTitle[]="STM32F4D-ChibiOS LCD GUI Demo";
Thread* mainThread;

__attribute__  ((noreturn))
static msg_t Thread1(void *arg) {
	(void)arg;
	chRegSetThreadName("blinker");

	while (TRUE)
	{
		key=getkey();
		ui_executecmd(key);
		chThdSleep(50);
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
	p;
	palTogglePad(GPIOD, GPIOD_LED3);
	chVTSetI(&vt1, 500, led_toggle, NULL);
}

// Entry Point
int main(void)
{
	halInit();
	chSysInit();

	mainThread=chThdSelf();

	hw_init();

	chThdCreateFromHeap(NULL, 1024, NORMALPRIO, Thread1, NULL);

	chVTSetI(&vt1, 500, led_toggle, NULL);

	while (TRUE) {
		ui_executecmd(0xFF);
		chThdSleep(100);
	}
}
