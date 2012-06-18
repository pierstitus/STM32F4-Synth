#include "CLCDQVGA_Driver.h"
#include "stm32f4xx.h"
#include "fonts.h"

#define LCD_RST_LOW		palClearPad(LCD_RST_GPIO, LCD_RST_PIN)
#define LCD_RST_HIGH	palSetPad(LCD_RST_GPIO, LCD_RST_PIN)

#define LCD_CS_LOW		palClearPad(LCD_CS_GPIO, LCD_CS_PIN)
#define LCD_CS_HIGH		palSetPad(LCD_CS_GPIO, LCD_CS_PIN)

#define LCD_RS_LOW		palClearPad(LCD_RS_GPIO, LCD_RS_PIN)
#define LCD_RS_HIGH		palSetPad(LCD_RS_GPIO, LCD_RS_PIN)

#define LCD_RD_LOW		palClearPad(LCD_RD_GPIO, LCD_RD_PIN)
#define LCD_RD_HIGH		palSetPad(LCD_RD_GPIO, LCD_RD_PIN)

#define LCD_WR_LOW		palClearPad(LCD_WR_GPIO, LCD_WR_PIN)
#define LCD_WR_HIGH		palSetPad(LCD_WR_GPIO, LCD_WR_PIN)

#define LCD_BL_LOW		palClearPad(LCD_BL_GPIO, LCD_BL_PIN)
#define LCD_BL_HIGH		palSetPad(LCD_BL_GPIO, LCD_BL_PIN)

#define USE_BITBANG

#define RAMWR			0x22

volatile uint16_t cx,cy;
volatile uint16_t px,py;
volatile uint16_t bgcolor=WHITE,fgcolor=BLACK;

volatile uint8_t lcd_blstate=LCD_BL_ON;
volatile uint16_t lcd_bltime=0;

const uint16_t colors[]={BLACK, RED, GREEN, BLUE, CYAN, MAGENTA, YELLOW, WHITE};

const uint8_t* font;

static VirtualTimer lcd_blvt;

inline void lcd_lld_delay(void)
{
	asm volatile ("nop");
	asm volatile ("nop");
}

inline void lcd_lld_write(uint16_t db)
{
	LCD_D4_GPIO->BSRR.W=((~db&0xFFF0)<<16)|(db&0xFFF0);
	LCD_D0_GPIO->BSRR.W=((~db&0x000F)<<16)|(db&0x000F);

	LCD_WR_LOW;
	lcd_lld_delay();
	LCD_WR_HIGH;
}

void lcd_cmd(uint16_t c)
{
	LCD_CS_LOW;
	LCD_RS_LOW;

	lcd_lld_write(c);

	LCD_RS_HIGH;
	LCD_CS_HIGH;
}

void lcd_data(uint16_t d)
{
	LCD_CS_LOW;
	lcd_lld_write(d);
	LCD_CS_HIGH;
}


uint16_t lcd_read(uint16_t addr)
{
	uint16_t retval;
	lcd_cmd(addr);

	LCD_RS_HIGH;
	asm volatile ("nop");
	asm volatile ("nop");

	palSetGroupMode(LCD_D0_GPIO, 0x0000000F, 0, PAL_MODE_INPUT);
	palSetGroupMode(LCD_D4_GPIO, 0x0000FFF0, 0, PAL_MODE_INPUT);

	LCD_RD_LOW;
	lcd_lld_delay();
	LCD_RD_HIGH;
	lcd_lld_delay();


	retval=(GPIOE->IDR&0xFFF0)|(GPIOD->IDR&0x000F);
	GPIOD->ODR=GPIOD->IDR;
	GPIOE->ODR=GPIOE->IDR;
	palSetGroupMode(LCD_D0_GPIO, 0x0000000F, 0, PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST);
	palSetGroupMode(LCD_D4_GPIO, 0x0000FFF0, 0, PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST);

	asm volatile ("nop");
	asm volatile ("nop");

	return retval;
}

void lcd_writereg(uint16_t c, uint16_t d)
{
	LCD_CS_LOW;
	LCD_RS_LOW;

	lcd_lld_write(c);

	LCD_RS_HIGH;

	lcd_lld_write(d);

	LCD_CS_HIGH;
}


void lcd_blup()
{
	if (lcd_blstate!=LCD_BL_ONN&&lcd_blstate!=LCD_BL_ON) {
		lcd_blstate=LCD_BL_ONN;
	} else {
		lcd_bltime=LCD_BACKLIGHT_ON_TIME;
	}
}

// Modified to Support ChibiOS VT API
void lcd_blhook(void *p)
{
	// Silence the compiler
	(void*) p;


	// Perform action of back light
	switch (lcd_blstate)
	{
		case LCD_BL_ON:
		case LCD_BL_DIM0:
			if (lcd_bltime==0) {
				lcd_blstate=LCD_BL_DIM0;
				TIM4->CCR3-=10;
				if (TIM4->CCR3<=100) {
					lcd_blstate=LCD_BL_DIM;
					lcd_bltime=LCD_BACKLIGHT_DIM_TIME;
				}
			} else {
				lcd_bltime--;
			}
			break;


		case LCD_BL_DIM:
			if (lcd_bltime==0) {
				if (TIM4->CCR3!=0) TIM4->CCR3--;
				else lcd_blstate=LCD_BL_OFF;
			} else {
				lcd_bltime--;
			}
			break;

		case LCD_BL_OFF:
			break;

		case LCD_BL_ONN:
			TIM4->CCR3+=20;
			if (TIM4->CCR3>239) {
				TIM4->CCR3=255;
				lcd_blstate=LCD_BL_ON;
				lcd_bltime=LCD_BACKLIGHT_ON_TIME;
			}
			/* no break */
	}

	// Re-arm the timer
	chVTSetI(&lcd_blvt, MS2ST(100), lcd_blhook, NULL);
}

// Initializes the LCD
void lcd_init()
{
	// IO Default Configurations
	palSetPadMode(LCD_CS_GPIO, LCD_CS_PIN, PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST);
	palSetPadMode(LCD_WR_GPIO, LCD_WR_PIN, PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST);
	palSetPadMode(LCD_RD_GPIO, LCD_RD_PIN, PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST);
	palSetPadMode(LCD_RST_GPIO, LCD_RST_PIN, PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST);
	palSetPadMode(LCD_RS_GPIO, LCD_RS_PIN, PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST);

	palSetGroupMode(LCD_D0_GPIO, 0x0000000F, 0, PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST);
	palSetGroupMode(LCD_D4_GPIO, 0x0000FFF0, 0, PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST);

	// Timer4 - Channel 3s
	palSetPadMode(LCD_BL_GPIO, LCD_BL_PIN, PAL_MODE_ALTERNATE(2));

	tsc_init();

	LCD_CS_HIGH;
	LCD_RST_HIGH;
	LCD_RD_HIGH;
	LCD_WR_HIGH;

	lcd_bltime=LCD_BACKLIGHT_ON_TIME;

	font = font_MediumBold;

	// Init the timer PWM without HAL
	// 256 Steps @ 31.25 kHz
	// Auto Load Configuration
	// TIM4 / CH3 (PB8)
	rccEnableTIM4(FALSE);
	TIM4->ARR = 255;

	// We prescale the clock to 8 MHz ( 8 MHz / 256 = 31.25 kHz)
	TIM4->PSC = (STM32_TIMCLK1 / 8000000) - 1;

	// Now configure OC3 in PWM Mode 1 OC3M[2:0] = 110
	// Enable preload
	TIM4->CCMR2 = TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3PE;

	// Enable OC4 output, active low
	TIM4->CCER = TIM_CCER_CC3E | TIM_CCER_CC3P;

	// Finally, enable auto reload and start the timer!
	TIM4->CR1 = TIM_CR1_ARPE | TIM_CR1_CEN;

	// Set initial brightness level
	TIM4->CCR3=255;

	chVTSetI(&lcd_blvt, MS2ST(100), lcd_blhook, NULL);

	LCD_RST_LOW;
	chThdSleepMilliseconds(2);
	LCD_RST_HIGH;	         // Hardware Reset
	chThdSleepMilliseconds(2);

	lcd_writereg(0x11,0x2004);
    lcd_writereg(0x13,0xCC00);
    lcd_writereg(0x15,0x2600);
	lcd_writereg(0x14,0x252A);
	lcd_writereg(0x12,0x0033);
	lcd_writereg(0x13,0xCC04);

	chThdSleepMilliseconds(1);

	lcd_writereg(0x13,0xCC06);

	chThdSleepMilliseconds(1);

	lcd_writereg(0x13,0xCC4F);

	chThdSleepMilliseconds(1);

	lcd_writereg(0x13,0x674F);
	lcd_writereg(0x11,0x2003);

	chThdSleepMilliseconds(1);

	// Gamma Setting
	lcd_writereg(0x30,0x2609);
	lcd_writereg(0x31,0x242C);
	lcd_writereg(0x32,0x1F23);
	lcd_writereg(0x33,0x2425);
	lcd_writereg(0x34,0x2226);
	lcd_writereg(0x35,0x2523);
	lcd_writereg(0x36,0x1C1A);
	lcd_writereg(0x37,0x131D);
	lcd_writereg(0x38,0x0B11);
	lcd_writereg(0x39,0x1210);
	lcd_writereg(0x3A,0x1315);
	lcd_writereg(0x3B,0x3619);
	lcd_writereg(0x3C,0x0D00);
	lcd_writereg(0x3D,0x000D);

	lcd_writereg(0x16,0x0007);
	lcd_writereg(0x02,0x0013);
	lcd_writereg(0x03,0x0003);
	lcd_writereg(0x01,0x0127);

	chThdSleepMilliseconds(1);

	lcd_writereg(0x08,0x0303);
	lcd_writereg(0x0A,0x000B);
	lcd_writereg(0x0B,0x0003);
	lcd_writereg(0x0C,0x0000);
	lcd_writereg(0x41,0x0000);
	lcd_writereg(0x50,0x0000);
	lcd_writereg(0x60,0x0005);
    lcd_writereg(0x70,0x000B);
	lcd_writereg(0x71,0x0000);
	lcd_writereg(0x78,0x0000);
	lcd_writereg(0x7A,0x0000);
	lcd_writereg(0x79,0x0007);
	lcd_writereg(0x07,0x0051);

	chThdSleepMilliseconds(1);

	lcd_writereg(0x07,0x0053);
	lcd_writereg(0x79,0x0000);
}

// Clears the screen
void lcd_cls(void)
{
	lcd_setrect(0,239,0,319);
	lcd_locate(0,0);
	lcd_cmd(RAMWR);

	LCD_CS_LOW;
	for (int i=0; i<320*240;i++) lcd_lld_write(bgcolor);
	LCD_CS_HIGH;
}

void lcd_setrect(uint16_t xs, uint16_t xe, uint16_t ys, uint16_t ye)
{
	lcd_writereg(0x46, (xe<<8)|xs);
	lcd_writereg(0x48, ys);
	lcd_writereg(0x47, ye);
}

void lcd_filledrect(uint16_t clr, uint16_t xs, uint16_t ys, uint16_t w, uint16_t h)
{
	lcd_setrect(xs,xs+w-1,ys,ys+h-1);
	lcd_locate(xs, ys);
	lcd_cmd(RAMWR);

	LCD_CS_LOW;
	for (int i=0;i<w*h;i++) {
		lcd_lld_write(clr);
	}
	LCD_CS_HIGH;
	lcd_setrect(0,239,0,319);
}

void lcd_putpixel(uint16_t x, uint16_t y, uint16_t color)
{
	lcd_writereg(0x20, x);
	lcd_writereg(0x21, y);
	lcd_writereg(0x22, color);
}

void lcd_putchar(char c)
{
	const uint8_t* ptr;

	uint8_t fontHeight=font[FONT_TABLE_HEIGHT_IDX];
	uint8_t sps=font[FONT_TABLE_PAD_AFTER_CHAR_IDX];

	uint16_t chi;

	if (c<0x20||c>0x7F) {
		if (c=='\n') lcd_newline();
		return;
	}

	chi=*(uint16_t*)(&font[FONT_TABLE_CHAR_LOOKUP_IDX+ ((c-0x20)<<1)]);

	ptr=font+chi;

	uint8_t fontWidth=*(ptr++);

	if (cx+fontWidth>240) lcd_newline();

	for (uint8_t x=0;x<fontWidth;x++) {
		chi=*(uint16_t*)ptr;

		for (uint8_t y=0;y<fontHeight;y++) {

			if (chi&0x01)
				lcd_putpixel(cx+x, cy+y, fgcolor);
			else
				lcd_putpixel(cx+x, cy+y, bgcolor);

			chi>>=1;
		}
		ptr+=2;
	}

	cx+=fontWidth;
	if (sps!=0) {
		lcd_filledrect(bgcolor,cx,cy,sps,fontHeight);
		cx+=sps;
	}
}

void lcd_newline(void)
{
	cx=3;
	cy+=*(font+FONT_TABLE_HEIGHT_IDX)+1;
}

void lcd_puts(char* c)
{
	char d;
	while ((d=*c++))
		lcd_putchar(d);
}

void tsc_init(void)
{
	palSetPadMode(GPIOC, 5, PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST);
	tsc_read(0xD0);
}

uint16_t tsc_read(uint8_t addr)
{
	uint16_t result;
	uint8_t txbuf[3]={0}, rxbuf[3]={0};

	txbuf[0]=addr;

	spiAcquireBus(&SPID1);

	SPI1->CR1&=~SPI_CR1_SPE;
	SPI1->CR1|=(SPI_CR1_BR_2);
	SPI1->CR1|=SPI_CR1_SPE;

	palClearPad(GPIOC, 5);

	spiExchange(&SPID1, 3, txbuf, rxbuf);

	palSetPad(GPIOC, 5);

	result=(rxbuf[1]<<8) | rxbuf[2];

	SPI1->CR1&=~SPI_CR1_SPE;
	SPI1->CR1&=~(SPI_CR1_BR_2);
	SPI1->CR1|=SPI_CR1_SPE;

	spiReleaseBus(&SPID1);

	result>>=3;
	return result;
}

int lcd_getpointraw(uint16_t* _x, uint16_t* _y)
{
	uint16_t x[7], y[7];

	tsc_read(0xD1);
	for (int i=0;i<7;i++)
		x[i]=tsc_read(0xD1);

	tsc_read(0x91);

	for (int i=0;i<7;i++)
		y[i]=tsc_read(0x91);

	tsc_read(0xD0);

	for (int i=0; i<6; i++)
	{
		for (int j=0; j<7; j++)
		{
			if (x[i]>x[j])
			{
				int t=x[i];
				x[i]=x[j];
				x[j]=t;
			}
			if (y[i]>y[j])
			{
				int t=y[i];
				y[i]=y[j];
				y[j]=t;
			}
		}
	}


	*_x=x[3];
	*_y=y[3];

	return 1;
}

int lcd_getpoint2(void)
{
	uint16_t x1=0, y1=0, x2=0, y2=0, dx, dy;

	tsc_read(0xD1);
	tsc_read(0x91);

	for (int i=0;i<8;i++)
	{
		x1+=tsc_read(0xD1);
		y1+=tsc_read(0x91);
	}

	for (int i=0;i<8;i++)
	{
		x2+=tsc_read(0xD1);
		y2+=tsc_read(0x91);
	}

	tsc_read(0x90);

	if (palReadPad(GPIOB, 0)) return 0;

	x1=x1/8;
	y1=y1/8;

	x2=x2/8;
	y2=y2/8;

	dx=x1 > x2 ? x1-x2 : x2-x1;
	dy=y1 > y2 ? y1-y2 : y2-y1;

	if (dx < 3 && dy < 3)
	{
		x1=(x1+x2)/2;
		y1=(y1+y2)/2;

		px=240-((x1-330)*10/142);
		py=320-((y1-380)/11);

		return 1;
	}
	return 0;
}

void lcd_getpoint_int(uint16_t* lx, uint16_t* ly);

int lcd_getpoint()
{
	uint16_t x1=0, y1=0, x2=0, y2=0, dx, dy;
	lcd_getpoint_int(&x1, &y1);
	lcd_getpoint_int(&x2, &y2);

	dx=x2 > x1 ? x2-x1 : x1-x2;
	dy=y2 > y1 ? y2-y1 : y1-y2;

	if (dx < 3 && dy < 3)
	{
		px=(x1+x2)/2;
		py=(y1+y2)/2;
		return 1;
	}

	else
		return 0;
}

void lcd_getpoint_int(uint16_t* lx, uint16_t* ly)
{
	uint16_t x1=0, y1=0;

	uint16_t x[7], y[7];

	if (!tsc_isPenDown()) return;

	tsc_read(0xD1);
	tsc_read(0x91);

	for (int i=0;i<7;i++)
	{
		x[i]=tsc_read(0xD1);
		y[i]=tsc_read(0x91);
	}

	tsc_read(0x90);

	for (int i=0; i<6; i++)
	{
		for (int j=0; j<7; j++)
		{
			if (x[i]>x[j])
			{
				int t=x[i];
				x[i]=x[j];
				x[j]=t;
			}
			if (y[i]>y[j])
			{
				int t=y[i];
				y[i]=y[j];
				y[j]=t;
			}
		}
	}

	x1=x[3];
	y1=y[3];

	*lx=240-((x1-330)*10/142);
	*ly=320-((y1-380)/11);
}
