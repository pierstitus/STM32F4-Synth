// File adopted from ChibiOS STM32F4Discovery Demo

#include "ch.h"
#include "hal.h"

#include "chprintf.h"
#include "lis302dl.h"

// User Includes
#include "codec.h"

#include "synth.h"
#include <math.h>

#include "usbcfg.h"

// next lines are necessary since synth_contol.cpp is included
// and are copied from ChibiOS/testhal/STM32F4xx/RTC/main.c
/* libc stub */
int _getpid(void) {return 1;}
/* libc stub */
void _exit(int i) {(void)i;while(1);}
/* libc stub */
#include <errno.h>
#undef errno
extern int errno;
int _kill(int pid, int sig) {
  (void)pid;
  (void)sig;
  errno = EINVAL;
  return -1;
}

// sqrtf which uses FPU, the standard one apparently doesn't
float vsqrtf(float op1) {
  float result;
  __ASM volatile ("vsqrt.f32 %0, %1" : "=w" (result) : "w" (op1) );
  return (result);
}

BaseSequentialStream* chp;

/* Virtual serial port over USB.*/
SerialUSBDriver SDU1;

static SerialConfig ser_cfg = {
    2000000,
    0,
    0,
    0,
};

/*
 * SPI1 configuration structure.
 * Speed 5.25MHz, CPHA=1, CPOL=1, 8bits frames, MSb transmitted first.
 * The slave select line is the pin GPIOE_CS_SPI on the port GPIOE.
 */
static const SPIConfig spi1cfg = {
  NULL,
  /* HW dependent part.*/
  GPIOE,
  GPIOE_CS_SPI,
  SPI_CR1_BR_0 | SPI_CR1_BR_1 | SPI_CR1_CPOL | SPI_CR1_CPHA
};
/*
 * This is a periodic thread that reads accelerometer and sends messages
 */
static WORKING_AREA(waThreadAccel, 128);
__attribute__  ((noreturn))
static msg_t ThreadAccel(void *arg) {\
  systime_t time;                   /* Next deadline.*/
  float acc_x, acc_y, acc_z, acc_abs;

  (void)arg;
  chRegSetThreadName("accel");

  /* LIS302DL initialization.*/
  lis302dlWriteRegister(&SPID1, LIS302DL_CTRL_REG1, 0b11000111); // 01000011 DR-0:100Hz, 1:400Hz PD-PowerDown FS-0:2g 1:8g STP STM Zen Yen Xen  // 0x43
  lis302dlWriteRegister(&SPID1, LIS302DL_CTRL_REG2, 0x00);
  lis302dlWriteRegister(&SPID1, LIS302DL_CTRL_REG3, 0x00);

  /* Reader thread loop.*/
  time = chTimeNow();
  while (TRUE) {
    /* Reading MEMS accelerometer X and Y registers.*/
    // Hard-coded calibration values, * 1.12 offset
    acc_x = ((float)(int8_t)lis302dlReadRegister(&SPID1, LIS302DL_OUTX))/64*1.0+0.0;
    acc_y = ((float)(int8_t)lis302dlReadRegister(&SPID1, LIS302DL_OUTY))/64*1.0+0.0;
    acc_z = ((float)(int8_t)lis302dlReadRegister(&SPID1, LIS302DL_OUTZ))/64*1.0+0.0;

    #define pow2(x) (x*x)
    acc_abs = vsqrtf(pow2(acc_x) + pow2(acc_y) + pow2(acc_z));
    if (acc_abs>0.001) {
      acc_x /= acc_abs;
      acc_y /= acc_abs;
      acc_z /= acc_abs;
    }
    *(synth_interface.acc_abs) = acc_abs;
    *(synth_interface.acc_x) = acc_x;
    *(synth_interface.acc_y) = acc_y;
    *(synth_interface.acc_z) = acc_z;

    /*
    chprintf(chp, "%4d %4d %4d %4d \r",
             (int)(1000*acc_x), 
             (int)(1000*acc_y),
             (int)(1000*acc_z),
             (int)(1000*acc_abs));
    */

    /* Waiting until the next 10 milliseconds time interval.*/
    chThdSleepUntil(time += MS2ST(2));
  }
}


// Hardware Initialization
static void hw_init(void)
{
	// Init Serial Port
	sdStart(&SD2, &ser_cfg);
	palSetPadMode(GPIOA, 2, PAL_MODE_ALTERNATE(7));
	palSetPadMode(GPIOA, 3, PAL_MODE_ALTERNATE(7));

  /* Initializes a serial-over-USB CDC driver. */
  sduObjectInit(&SDU1);
  sduStart(&SDU1, &serusbcfg);

  /*
   * Initializes the SPI driver 1 in order to access the MEMS. The signals
   * are already initialized in the board file.
   */
  spiStart(&SPID1, &spi1cfg);

  /*
   * Activates the USB driver and then the USB bus pull-up on D+.
   * Note, a delay is inserted in order to not have to disconnect the cable
   * after a reset.
   */
  usbDisconnectBus(serusbcfg.usbp);
  chThdSleepMilliseconds(1000);
  usbStart(serusbcfg.usbp, &usbcfg);
  usbConnectBus(serusbcfg.usbp);

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

	chThdSelf();

	hw_init();

  chp = (BaseSequentialStream *)&SDU1; // output for chprintf

	codec_i2s_init(44100, 16);

	start_synth_thread();
	chThdSleep(10);

	chThdCreateStatic(waThreadAccel, sizeof(waThreadAccel), NORMALPRIO, ThreadAccel, NULL);

	chVTSetI(&vt1, 500, led_toggle, NULL);

	while (TRUE) {
		chThdSleep(100);
	}
}
