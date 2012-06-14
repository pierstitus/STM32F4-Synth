/*
 * codec.h
 *
 *  Created on: Jun 7, 2012
 *      Author: Kumar Abhishek
 */

#ifndef CODEC_H_
#define CODEC_H_

#include "ch.h"
#include "hal.h"

#define CODEC_I2C				I2CD1
#define CODEC_I2S_ENABLE		rccEnableSPI3(FALSE)
#define CODEC_I2S_DISABLE		rccDisableSPI3(FALSE)
#define CODEC_I2S				SPI3

enum CODEC_AUDIOSRC {
	CODEC_AUDIOSRC_DIGITAL,
	CODEC_AUDIOSRC_MIC,
	CODEC_AUDIOSRC_FMRADIO
};


#define CS43L22_ADDR	(0x94 >> 1)

extern void codec_hw_init(void);

extern void codec_hw_reset(void);

extern void codec_writeReg(uint8_t addr, uint8_t data);

extern uint8_t codec_readReg(uint8_t addr);

extern void codec_volCtl(uint8_t vol);

extern void codec_pwrCtl(uint8_t pwr);

extern void codec_muteCtl(uint8_t mute);

extern void codec_sendBeep(void);

extern void codec_selectAudioSource(uint8_t src);

extern void codec_i2s_init(uint16_t sampleRate, uint8_t nBits);

extern void codec_audio_send(void* txbuf, size_t n);

extern void codec_pauseResumePlayback(uint8_t pause);

#endif /* CODEC_H_ */
