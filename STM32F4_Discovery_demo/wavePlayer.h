/*
 * wavePlayer.h
 *
 *  Created on: Jun 8, 2012
 *      Author: Kumar Abhishek
 */

#ifndef WAVEPLAYER_H_
#define WAVEPLAYER_H_

#include "ch.h"
#include "hal.h"

#include "UI.h"
#include "ff.h"

#include "codec.h"

#include "chprintf.h"
#include "xprintf.h"

#define READ_UINT32(ptr)		*((uint32_t*)ptr)
#define READ_UINT16(ptr)		*((uint16_t*)ptr)

#define WAVE_HEAD_RIFF			0x46464952		// 'FIRR' = RIFF in Little-Endian format
#define WAVE_HEAD_WAVE			0x45564157		// WAVE in Little-Endian
#define WAVE_HEAD_DATA			0x61746164		// 'data' in Little-Endian
#define WAVE_HEAD_FMT			0x20746D66

#define WAVE_META_LIST			0x5453494C
#define WAVE_META_INFO			0x4F464E49
#define WAVE_META_INAM			0x4D414E49
#define WAVE_META_IART			0x54524149
#define WAVE_META_ICMT			0x544D4349
#define WAVE_META_ICRD			0x44524349

typedef struct _wavfile {
	uint32_t riffSignature;
	uint32_t fileSize;
	uint32_t waveSignature;
	uint32_t fmtSignature;
	uint32_t subChunk1Size;
	uint16_t format;
	uint16_t numChannels;
	uint32_t sampleRate;
	uint32_t byteRate;
	uint16_t blockAlign;
	uint16_t numBits;
} WAVFILE;

extern Thread* playerThread;

#endif /* WAVEPLAYER_H_ */
