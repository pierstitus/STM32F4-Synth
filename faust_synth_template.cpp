
extern "C" {
    #include "synth.h"
}

// definitions for max and min used by faust. Not sure if this is optimal when
// there are complex constructions inside.
#define max(x,y) (x>y?x:y)
#define min(x,y) (x<y?x:y)

// faster than using powf, probably the faustpower template can be extended to
// to faustpower<int>(float x)
#define faustpower3(x) ((x)*(x)*(x))
#define faustpower2(x) ((x)*(x))

// minimal DSP class
class dsp {
	protected:
		int fSamplingFreq;
};

// minimal Meta class
struct Meta {
    virtual void declare(const char* key, const char* value) = 0;
};

// variables
synth_interface_t synth_interface;

static int16_t buf1[PLAYBACK_BUFFER_SIZE] = {0};
static int16_t buf2[PLAYBACK_BUFFER_SIZE] = {0};
static float output0[CHANNEL_BUFFER_SIZE] = {0.0};
static float output1[CHANNEL_BUFFER_SIZE] = {0.0};
static float* output[2] = {output0, output1};

// Intrinsics
<<includeIntrinsic>>

// Class
<<includeclass>>

FAUSTCLASS dsp;

static WORKING_AREA(waSynthThread, 1024);
static msg_t synthThread(void *arg) {  // THE SYNTH THREAD
	(void)arg;
	chRegSetThreadName("SYNTH");

	uint_fast8_t bufSwitch=1;
	int16_t* buf = buf1;
	int32_t tmp;
	int count = CHANNEL_BUFFER_SIZE;
	int n = 0;

	codec_pwrCtl(1);    // POWER ON
	codec_muteCtl(0);   // MUTE OFF

	chEvtAddEvents(1);

	// initialization
	dsp.init(SAMPLINGFREQ);

	// initialize interface
	dsp.buildUserInterfaceEmbedded();

	// computation loop
	while (true) {
		dsp.compute(count, NULL, output);

		// double buffering
		if (bufSwitch) {
			buf = buf1;
			bufSwitch=0;
		} else {
			buf = buf2;
			bufSwitch=1;
		}
		// convert float to int with scale, clamp and round
		for (int n = 0; n < CHANNEL_BUFFER_SIZE; n++) {
			tmp = (int32_t)(output0[n] * 32768);
			tmp = (tmp <= -32768) ? -32768 : (tmp >= 32767) ? 32767 : tmp;
			// enable LED on clip
			//if (tmp <= -32768 || tmp >= 32767)
			//{
			//	palSetPad(GPIOD, GPIOD_LED3);       /* Orange.  */
			//} else {
			//	palClearPad(GPIOD, GPIOD_LED3);       /* Orange.  */
			//}
			// make both audio channels the same
			buf[2*n] = buf[2*n+1] = (int16_t)tmp;
		}

		if (--n <= 0) {
			palTogglePad(GPIOD, GPIOD_LED3);       /* Orange.  */
			n = 100;
		}

		chEvtWaitOne(1);
		codec_audio_send(buf, PLAYBACK_BUFFER_SIZE);

		if (chThdShouldTerminate()) break;
	}

	codec_muteCtl(1);
	codec_pwrCtl(0);

	audioThread=NULL;
	palTogglePad(GPIOD, GPIOD_LED5);

	return 0;
};

void start_synth_thread(void) {
	audioThread = chThdCreateStatic(waSynthThread, sizeof(waSynthThread), NORMALPRIO+2, synthThread, NULL);
}

