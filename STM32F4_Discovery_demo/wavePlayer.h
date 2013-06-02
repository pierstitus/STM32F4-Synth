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
#include "codec.h"
#include "chprintf.h"

#define READ_UINT32(ptr)		*((uint32_t*)ptr)
#define READ_UINT16(ptr)		*((uint16_t*)ptr)

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
