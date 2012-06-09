/*
 * wavePlayer.c
 *
 *  Created on: Jun 7, 2012
 *      Author: Kumar Abhishek
 */

#include "wavePlayer.h"

static WORKING_AREA(waThread3, 1024);

extern uint8_t buf[];
extern uint8_t buf2[];

uint32_t waveSampleLength=0, bytesToPlay;

Thread* playerThread;

FIL f1;

extern stm32_dma_stream_t* i2sdma;

static void wavePlayEventHandler(uint8_t evt)
{
	if (evt && evt < 11)
	{
		lcd_cls();
		ui_displayPreviousMenu();
	}
}

static msg_t Thread2(void *arg) {
	(void)arg;
	chRegSetThreadName("blinker");

	UINT temp, flag=0, pause=0;

	f_read(&f1, buf2, 2048, &temp);
	bytesToPlay-=temp;

	codec_pwrCtl(1);
	codec_muteCtl(0);

	uint16_t vol=200;

	while(bytesToPlay)
	{
		if (!pause)
		{
			dmaWaitCompletion(i2sdma);
			if (flag)
			{
				codec_audio_send(buf, temp/2);
				f_read(&f1, buf2, 2048, &temp);
				flag=0;
			}
			else
			{
				codec_audio_send(buf2, temp/2);
				f_read(&f1, buf, 2048, &temp);
				flag=1;
			}

			bytesToPlay-=temp;
		}

		key=getkey();

		if (key==BTN_MID_DOWN) {
			vol--;
			if (vol < 150) vol=150;
			codec_volCtl(vol);
		}
		else if (key==BTN_MID_UP) {
			vol++;
			if (vol > 220) vol=220;
			codec_volCtl(vol);
		}
		else if (key==BTN_MID_SEL) {
			if (!pause)
			{
				pause=1;
				codec_muteCtl(1);
				CODEC_I2S->CR2=0;
				dmaStreamDisable(i2sdma);

				chThdSleepMilliseconds(20);
			}
			else
			{
				pause=0;
				CODEC_I2S->CR2=SPI_CR2_TXDMAEN;
				dmaStreamEnable(i2sdma);
				codec_muteCtl(0);
				chThdSleepMilliseconds(20);
			}
		}
		else if (key==BTN_RIGHT) {
			break;
		}
	}

	codec_muteCtl(1);
	codec_pwrCtl(0);

	f_close(&f1);

	return 0;
}



void playWaveFile(void)
{

	uint8_t st;
	UINT btr;
	WAVFILE* wf;

	uint32_t temp, offset=44, cnt;

	ui_clearUserArea();
	ui_drawTitleBar("Play Wave File");
	ui_sethandler((pfn)wavePlayEventHandler);

	cx=3; cy=UI_TBARHEIGHT+3;

	st=f_open(&f1, "audio.wav", FA_READ);

	if (st) {
		lcd_puts("Invalid file");
		return;
	}

	st=f_read(&f1, buf, 1024, &btr);

	wf=(WAVFILE*)&buf[0];

	if (wf->riffSignature != WAVE_HEAD_RIFF || wf->fmtSignature != WAVE_HEAD_FMT || wf->format!=0x01) {
		lcd_puts("This is a not a wave file!");
		return;
	}

	xprintf("Number of channels=%d\nSample Rate=%ld\nNumber of Bits=%d\n", wf->numChannels, wf->sampleRate, wf->numBits);

	temp=READ_UINT32(&buf[36]);

	if (temp==WAVE_HEAD_DATA)
	{
		// Got 'data' chunk, ready to play
		waveSampleLength=READ_UINT32(&buf[40]);
		lcd_puts("No Metadata. Ready to play.\n");
		offset=44;
	}
	else if (temp==WAVE_META_LIST)
	{
		uint32_t msize;
		lcd_puts("File has metadata:");
		msize=READ_UINT32(&buf[40]);
		offset=52+msize;
		xprintf("%ld bytes.\nActual Data starts at offset:%ld\n", msize, offset);

		cnt=0;
		temp=READ_UINT32(&buf[44+cnt]);
		// Tag Reading Supported only on Audacity Output
		if (temp==WAVE_META_INFO)
		{
			cnt+=4;
			do
			{
				temp=READ_UINT32(&buf[44+cnt]);

				switch (temp)
				{
					case WAVE_META_INAM:
						cnt+=4;
						temp=READ_UINT32(&buf[44+cnt]);
						cnt+=4;
						xprintf("Name: %s\n", &buf[44+cnt]);
						cnt+=temp;
						break;

					case WAVE_META_IART:
						cnt+=4;
						temp=READ_UINT32(&buf[44+cnt]);
						cnt+=4;
						xprintf("Artist: %s\n", &buf[44+cnt]);
						cnt+=temp;
						break;

					case WAVE_META_ICMT:
						cnt+=4;
						temp=READ_UINT32(&buf[44+cnt]);
						cnt+=4;
						xprintf("Composer: %s\n", &buf[44+cnt]);
						cnt+=temp;
						break;

					case WAVE_META_ICRD:
						cnt+=4;
						temp=READ_UINT32(&buf[44+cnt]);
						cnt+=4;
						xprintf("Release Yr: %s\n", &buf[44+cnt]);
						cnt+=temp;
						break;

					default:
						cnt+=8;			// Token+Size (4 bytes each)
						break;
				}

			} while (cnt < msize);
			waveSampleLength=READ_UINT32(&buf[offset-4]);
			lcd_puts("Ready To Play.\n");
		}
	}

	codec_i2s_init(wf->sampleRate, wf->numBits);

	xprintf("Sample Length:%ld bytes", waveSampleLength);

	bytesToPlay=waveSampleLength;

	f_lseek(&f1, offset);

	playerThread=chThdCreateStatic(waThread3, sizeof(waThread3), NORMALPRIO, Thread2, NULL);
	chThdWait(playerThread);
	ui_executecmd(BTN_RIGHT);
}
