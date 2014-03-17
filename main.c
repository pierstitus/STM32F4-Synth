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

#define PLAYBACK_BUFFER_SIZE	256
int16_t buf1[PLAYBACK_BUFFER_SIZE]={0};
int16_t buf2[PLAYBACK_BUFFER_SIZE]={0};
float buf_f[PLAYBACK_BUFFER_SIZE] = {0.0};

static WORKING_AREA(waThread2, 256);
static msg_t synthThread(void *arg) {  // THE SYNTH THREAD
	(void)arg;
	chRegSetThreadName("SYNTH");

	uint_fast8_t bufSwitch=1;
	int16_t* buf = buf1;
	int32_t tmp;

	unsigned int n;

	float d = 0;
	float damp = 0.3;

	buf_f[0] = 200;

	codec_pwrCtl(1);    // POWER ON
	codec_muteCtl(0);   // MUTE OFF

	chEvtAddEvents(1);

	// start with a square wave
	for (n = 0; n < PLAYBACK_BUFFER_SIZE; n++)
	{
		buf_f[n] = (2.0*(n<128)-1.0);
	}

	while(1)
	{
		// do Karplus Strong filtering
		for (n = 0; n < PLAYBACK_BUFFER_SIZE; n++)
		{
			d = damp * buf_f[n] + (1-damp) * d;
			buf_f[n] = d;
		}

		// double buffering
		if (bufSwitch)
		{
			buf = buf1;
			bufSwitch=0;
		}
		else
		{
			buf = buf2;
			bufSwitch=1;
		}
		// convert float to int with scale, clamp and round
		for (n = 0; n < PLAYBACK_BUFFER_SIZE; n++)
		{
			tmp = (int32_t)(buf_f[n] * 32768);
			tmp = (tmp <= -32768) ? -32768 : (tmp >= 32767) ? 32767 : tmp;
			// enable LED on clip
			if (tmp <= -32768 || tmp >= 32767)
			{
				palSetPad(GPIOD, GPIOD_LED3);       /* Orange.  */
			} else {
				palClearPad(GPIOD, GPIOD_LED3);       /* Orange.  */
			}
			buf[n] = (int16_t)tmp;
		}

		chEvtWaitOne(1);
		codec_audio_send(buf, PLAYBACK_BUFFER_SIZE);

		if (chThdShouldTerminate()) break;
	}

	codec_muteCtl(1);
	codec_pwrCtl(0);

	playerThread=NULL;
	palTogglePad(GPIOD, GPIOD_LED5);

	return 0;
}


/*
 * This is a periodic thread that does absolutely nothing except flashing
 * a LED and beeping.
 */
static WORKING_AREA(waThread1, 128);
__attribute__  ((noreturn))
static msg_t Thread1(void *arg) {
	(void)arg;
	chRegSetThreadName("blinker");

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

	codec_i2s_init(44100, 16);

	playerThread=chThdCreateStatic(waThread2, sizeof(waThread2), NORMALPRIO, synthThread, NULL);
	//chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);

	chVTSetI(&vt1, 500, led_toggle, NULL);

	while (TRUE) {
		chThdSleep(100);
	}
}
