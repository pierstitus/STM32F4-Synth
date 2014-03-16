/*
 * wavePlayer.c
 *
 *  Created on: Jun 7, 2012
 *      Author: Kumar Abhishek
 */

#include "wavePlayer.h"

static WORKING_AREA(waThread3, 1024);
#define PLAYBACK_BUFFER_SIZE	512
uint8_t buf[PLAYBACK_BUFFER_SIZE]={0};
uint8_t buf2[PLAYBACK_BUFFER_SIZE]={0};
uint32_t waveSampleLength=0, bytesToPlay;
Thread* playerThread;
FIL f1;
extern stm32_dma_stream_t* i2sdma;
uint16_t vol=200;
uint8_t pause=0;

static void wavePlayEventHandler(uint8_t evt)
{
	static uint8_t prevEvt=0;

	if (evt && evt < 15)
		if (prevEvt!=evt) {
			prevEvt=evt;
			codec_sendBeep();
		}

	switch (evt)
	{
		case BTN_RIGHT:
			lcd_cls();
			ui_displayPreviousMenu();
			break;

		case BTN_LEFT:  // END PLAYBACK
			chThdTerminate(playerThread);
			chThdWait(playerThread);
			playerThread=NULL;
			break;

		case BTN_MID_DOWN:  // VOLUME UP
			vol--;
			if (vol < 150) vol=150;
			codec_volCtl(vol);
			break;

		case BTN_MID_UP:    // VOLUME DOWN
			vol++;
			if (vol > 220) vol=220;
			codec_volCtl(vol);
			break;

		case BTN_MID_SEL:   // PAUSE PLAYBACK
			if (!pause) {
				pause=1;
				ui_drawBottomBar("Stop", "Play", "Exit");
			} else {
				pause=0;
				ui_drawBottomBar("Stop", "Pause", "Exit");
			}
			codec_pauseResumePlayback(pause);
			break;
	}
}

static msg_t wavePlayerThread(void *arg) {  // THE PLAYER THREAD
	(void)arg;
	chRegSetThreadName("PLAYER");

	UINT temp, bufSwitch=1;

	codec_pwrCtl(1);    // POWER ON
	codec_muteCtl(0);   // MUTE OFF

	f_read(&f1, buf, PLAYBACK_BUFFER_SIZE, &temp);
	bytesToPlay-=temp;

	chEvtAddFlags(1);

	while(bytesToPlay)
	{
		chEvtWaitOne(1);

		if (bufSwitch)
		{
			codec_audio_send(buf, temp/2);
			spiAcquireBus(&SPID1);
			f_read(&f1, buf2, PLAYBACK_BUFFER_SIZE, &temp);
			spiReleaseBus(&SPID1);
			bufSwitch=0;
		}
		else
		{
			codec_audio_send(buf2, temp/2);
			spiAcquireBus(&SPID1);
			f_read(&f1, buf, PLAYBACK_BUFFER_SIZE, &temp);
			spiReleaseBus(&SPID1);
			bufSwitch=1;
		}

		bytesToPlay-=temp;

		if (chThdShouldTerminate()) break;
	}

	codec_muteCtl(1);
	codec_pwrCtl(0);

	playerThread=NULL;

	f_close(&f1);

	return 0;
}

void playWaveFile(void)  // Initializes the Thread
{

	uint8_t st;
	UINT btr;
	WAVFILE* wf;

	uint32_t temp, offset=44, cnt;

	ui_sethandler((pfn)wavePlayEventHandler);

	st=f_read(&f1, buf, 1024, &btr);

	wf=(WAVFILE*)&buf[0];

	codec_i2s_init(wf->sampleRate, wf->numBits);

	bytesToPlay=waveSampleLength;

	f_lseek(&f1, offset);

	playerThread=chThdCreateStatic(waThread3, sizeof(waThread3), NORMALPRIO+2, wavePlayerThread, NULL);
}
