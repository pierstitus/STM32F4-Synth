/*
 * codec_CS43L22.c
 *
 *  Created on: Jun 7, 2012
 *      Author: Kumar Abhishek
 */

#include "codec.h"

#include "chprintf.h"

const stm32_dma_stream_t* i2sdma;
static uint32_t i2stxdmamode=0;

extern Thread* playerThread;

static const I2CConfig i2cfg = {
    OPMODE_I2C,
    100000,
    FAST_DUTY_CYCLE_16_9,
};

#define I2S3_TX_DMA_CHANNEL                                                 \
  STM32_DMA_GETCHANNEL(STM32_SPI_SPI3_TX_DMA_STREAM,                        \
                       STM32_SPI3_TX_DMA_CHN)

static uint8_t txbuf[2]={0}, rxbuf[2]={0};

void codec_hw_init(void)
{
	// Fortunately, all pins have already been initialized in board.h

	// Start the i2c driver
	i2cStart(&CODEC_I2C, &i2cfg);

	// Reset the codec
	codec_hw_reset();

	// Write init sequence
	// Keep codec powered down initially
	codec_pwrCtl(0);

	codec_muteCtl(0);

	// Auto Detect Clock, MCLK/2
	codec_writeReg(0x05, 0x81);

	// Slave Mode, I2S Data Format
	codec_writeReg(0x06, 0x04);

	codec_pwrCtl(1);

	codec_volCtl(200);

	// Adjust PCM Volume
	codec_writeReg(0x1A, 0x0A);
	codec_writeReg(0x1B, 0x0A);

	// Disable the analog soft ramp
	codec_writeReg(0x0A, 0x00);

	// Disable the digital soft ramp
	codec_writeReg(0x0E, 0x04);

	// Disable the limiter attack level
	codec_writeReg(0x27, 0x00);

	codec_writeReg(0x1C, 0x80);
}

void codec_hw_reset(void)
{
	palClearPad(GPIOD, GPIOD_RESET);
	halPolledDelay(MS2RTT(10));
	palSetPad(GPIOD, GPIOD_RESET);
}

static void dma_i2s_interrupt(void* dat, uint32_t flags)
{
	dmaStreamDisable(i2sdma);

	chSysLockFromIsr();
	chEvtSignalFlagsI(playerThread, 1);
	chSysUnlockFromIsr();
}

static void codec_dma_init(void)
{
	i2sdma=STM32_DMA_STREAM(STM32_SPI_SPI3_TX_DMA_STREAM);

	i2stxdmamode = STM32_DMA_CR_CHSEL(I2S3_TX_DMA_CHANNEL) |
					STM32_DMA_CR_PL(STM32_SPI_SPI3_DMA_PRIORITY) |
					STM32_DMA_CR_DIR_M2P |
					STM32_DMA_CR_DMEIE |
					STM32_DMA_CR_TEIE |
					STM32_DMA_CR_TCIE |
					STM32_DMA_CR_PSIZE_HWORD | STM32_DMA_CR_MSIZE_HWORD;

	bool_t b = dmaStreamAllocate(i2sdma,
			STM32_SPI_SPI3_IRQ_PRIORITY,
			(stm32_dmaisr_t)dma_i2s_interrupt,
			(void *)&SPID3);

	if (!b)
		chprintf((BaseChannel*)&SD2, "DMA Allocated Successfully to I2S3\r\n");

	dmaStreamSetPeripheral(i2sdma, &(SPI3->DR));
}

void codec_i2s_init(uint16_t sampleRate, uint8_t nBits)
{
	uint16_t prescale;
	uint32_t pllfreq=STM32_PLLI2SVCO / STM32_PLLI2SR_VALUE;

	if (nBits!=16)
		return;

	// SPI3 in I2S Mode, Master
	CODEC_I2S_ENABLE;

	CODEC_I2S->I2SCFGR=SPI_I2SCFGR_I2SMOD | SPI_I2SCFGR_I2SCFG_1;

	// Master clock mode Fs * 256
	prescale=(pllfreq*10)/(256*sampleRate) + 5;
	prescale/=10;

	chprintf((BaseChannel*)&SD2, "Prescale value:%d\r\n", prescale);

	if (prescale > 0xFF || prescale < 2) prescale=2;

	if (prescale & 0x01)
		CODEC_I2S->I2SPR=SPI_I2SPR_MCKOE | SPI_I2SPR_ODD | (prescale>>1);
	else
		CODEC_I2S->I2SPR=SPI_I2SPR_MCKOE | (prescale>>1);

	codec_dma_init();

	// Enable I2S DMA Request
	CODEC_I2S->CR2  = SPI_CR2_TXDMAEN;

	// Now Enable I2S
	CODEC_I2S->I2SCFGR|=SPI_I2SCFGR_I2SE;
}

void codec_writeReg(uint8_t addr, uint8_t data)
{
	txbuf[0]=addr;
	txbuf[1]=data;
	i2cMasterTransmitTimeout(&I2CD1, CS43L22_ADDR, txbuf, 2, NULL, 0, MS2ST(4));
}

uint8_t codec_readReg(uint8_t addr)
{
	txbuf[0]=addr;
	i2cMasterTransmitTimeout(&I2CD1, CS43L22_ADDR, txbuf, 1, rxbuf, 2, MS2ST(4));
	return rxbuf[0];
}

void codec_pwrCtl(uint8_t pwr)
{
	if (pwr)
		codec_writeReg(0x02, 0x9E);
	else
		codec_writeReg(0x02, 0x01);
}

void codec_muteCtl(uint8_t mute)
{
	if (mute)
		codec_writeReg(0x04, 0xFF);
	else
		codec_writeReg(0x04, 0xAF);
}

void codec_volCtl(uint8_t vol)
{
	if (vol > 0xE6)
	{
		codec_writeReg(0x20, vol-0xE7);
		codec_writeReg(0x21, vol-0xE7);
	}
	else
	{
		codec_writeReg(0x20, vol+0x19);
		codec_writeReg(0x21, vol+0x19);
	}
}

void codec_selectAudioSource(uint8_t src)
{
	switch (src) {
		case CODEC_AUDIOSRC_DIGITAL:
			// Disable all pass through channels
			codec_writeReg(0x0E, 0x04);
			break;

		case CODEC_AUDIOSRC_MIC:
			// Select AIN4A/B for pass through
			codec_writeReg(0x08, 0x08);
			codec_writeReg(0x09, 0x08);
			codec_writeReg(0x0E, 0xC4);
			break;

		case CODEC_AUDIOSRC_FMRADIO:
			// Select AIN2A/B for pass through
			codec_writeReg(0x08, 0x02);
			codec_writeReg(0x09, 0x02);
			codec_writeReg(0x0E, 0xC4);
			break;
	}
}

// Send data to the codec via I2S
void codec_audio_send(void* txbuf, size_t n)
{
	dmaStreamSetMemory0(i2sdma, txbuf);
	dmaStreamSetTransactionSize(i2sdma, n);
	dmaStreamSetMode(i2sdma, i2stxdmamode | STM32_DMA_CR_MINC);
	dmaStreamClearInterrupt(i2sdma);
	dmaStreamEnable(i2sdma);
}

void codec_pauseResumePlayback(uint8_t pause)
{
	if (pause) {
		codec_muteCtl(1);
		codec_pwrCtl(0);

		CODEC_I2S->CR2=0;

	} else {
		codec_pwrCtl(1);

		CODEC_I2S->CR2=SPI_CR2_TXDMAEN;

		codec_muteCtl(0);
	}
}

void codec_sendBeep(void)
{
	codec_writeReg(0x1E, 0x00);
	codec_writeReg(0x1E, 0x40);
}
