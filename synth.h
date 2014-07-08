
#ifndef SYNTH_H_
#define SYNTH_H_

#include "ch.h"
#include "hal.h"

#include "codec.h"

#define SAMPLINGFREQ 44100
#define CHANNEL_BUFFER_SIZE		128
#define PLAYBACK_BUFFER_SIZE	(CHANNEL_BUFFER_SIZE*2)

typedef struct struct_synth_interface {
	float* acc_abs;
	float* acc_x;
	float* acc_y;
	float* acc_z;
} synth_interface_t;

extern synth_interface_t synth_interface;
extern synth_interface_t synth_interface_bas;

extern void start_synth_thread(void);

#endif /* SYNTH_H_ */