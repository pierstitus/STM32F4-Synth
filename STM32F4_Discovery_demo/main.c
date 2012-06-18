// File adopted from ChibiOS STM32F4Discovery Demo

#include "ch.h"
#include "hal.h"

#include "lis302dl.h"
#include "chprintf.h"
#include "xprintf.h"
#include "ff.h"

#include "UI.h"

// User Includes
#include "drawingApp.h"
#include "wavePlayer.h"

#define USE_MMC_CARD

#ifdef USE_MMC_CARD

// SPI SD Card
FATFS MMC_FS;

MMCDriver MMCD1;

static volatile bool_t fs_ready = FALSE;

void display_image(char* fn, uint16_t x, uint16_t y);

#endif

void mainEventHandler(uint8_t evt);
void displayStartUpScreen(void);
void displayMainScreen(void);

static void testRGB1(void);
static void testRGB2(void);
static void testRGB3(void);
static void testEventHandler(uint8_t evt);

static void testImages(void);

static void displayAboutScreen(void);

void playWaveFile(void);

// Menu Table
const menuItem subMenu1[]= {
	{MENU_HEADER, "Menu", (uint8_t*)displayMainScreen},
	{MENU_ITEM, "Test RGB Screen 1", (uint8_t*)testRGB1},
	{MENU_ITEM, "Test RGB Screen 2", (uint8_t*)testRGB2},
	{MENU_ITEM, "Test RGB Screen 3", (uint8_t*)testRGB3},
	{MENU_ITEM, "Test Images", (uint8_t*)testImages},
	{MENU_ITEM, "Play Wave File", (uint8_t*)playWaveFile},
	{MENU_ITEM, "Draw", (uint8_t*)startDrawingThread},
	{MENU_ITEM, "About", (uint8_t*)displayAboutScreen},
	{MENU_END, 0, 0}
};

// MMC Card Code
static SPIConfig hs_spicfg = {NULL, GPIOC, 4, SPI_CR1_BR_0};
#ifdef USE_MMC_CARD
// High Speed and Low Speed Configuration
static SPIConfig ls_spicfg = {NULL, GPIOC, 4, SPI_CR1_BR_2 | SPI_CR1_BR_1 };

// We don't have checking currently
static bool_t mmc_is_inserted(void) { return TRUE; }
static bool_t mmc_is_protected(void) {return FALSE; }

// MMC Card Insertion Event Handler
static void InsertHandler(eventid_t id)
{
	FRESULT err;
	(void) id;

	// Try establishing link with device
	if(mmcConnect(&MMCD1))
	{
		chprintf((BaseChannel *) &SD2, "SD: Failed to connect to card\r\n");
		return;
	}
	else
	{
		chprintf((BaseChannel *) &SD2, "SD: Connected to card\r\n");
	}

	// If connected, mount the volume
	err = f_mount(0, &MMC_FS);
	if(err != FR_OK)
	{
		// Successful
		chprintf((BaseChannel *) &SD2, "SD: f_mount() failed %d\r\n", err);
		mmcDisconnect(&MMCD1);
		return;
	}
	else
	{
		chprintf((BaseChannel *) &SD2, "SD: File system mounted\r\n");
	}

	// Set Ready Flag
	fs_ready = TRUE;
}

// MMC Card Removal Event Handler
static void RemoveHandler(eventid_t id)
{
	(void)id;
	fs_ready = FALSE;
}

// Display a bitmap image from the SD Card
void display_bitmap(char* fn, uint16_t x, uint16_t y)
{
	FIL fil;

	uint16_t s;
	uint16_t w,h,sz;
	UINT dmy, i;

	uint8_t* buffer;

	buffer=chHeapAlloc(NULL, 16384);

	s=f_open(&fil, fn, FA_READ);

	cx=3; cy=3;

	if (!buffer) {
		lcd_puts("Buffer allocation failed");
		return;
	}

	if (s==FR_NO_FILE) 	{
		lcd_puts("File does not exist.");
		return;
	}
	else if (s==FR_NOT_READY) {
		lcd_puts("Card not Inserted.");
		return;
	}

	// Read the header first
	f_read(&fil, buffer, 54, &dmy);

	s=*((uint16_t*)&buffer[0]);

	// Support only 24bpp bitmap
	if (s != 0x4D42 || buffer[28] != 24) {
		lcd_puts("Not a valid bitmap!");
		return;
	}

	w=*((uint16_t*)&buffer[18]);
	h=*((uint16_t*)&buffer[22]);

	sz=16384-54;

	lcd_setrect(x, x+w-1, y, y+h-1);
	lcd_writereg(0x03, 0x01);

	lcd_locate(x,y+h-1);

	lcd_cmd(0x22);

	palClearPad(LCD_CS_GPIO, LCD_CS_PIN);

	uint8_t clr[3], cnt=0;
	do
	{
		f_read(&fil, buffer, sz, &dmy);

		for (i=0;i<dmy;i++)
		{
			clr[cnt++]=buffer[i];
			if (cnt==3) {
				uint16_t index=(uint16_t)( (( clr[2] >> 3 ) << 11 ) | (( clr[1] >> 2 ) << 5  ) | ( clr[0]  >> 3 ));

				lcd_lld_write(index);
				cnt=0;
			}
		}

		if (dmy<sz) break;

		sz=16384;
	} while (1);

	palSetPad(LCD_CS_GPIO, LCD_CS_PIN);

	lcd_setrect(0, 239, 0, 319);
	lcd_locate(0,0);

	chHeapFree(buffer);

	// Close file and return
	f_close(&fil);
}



void display_image(char* fn, uint16_t x, uint16_t y) {
	FIL fil;

	uint8_t s;
	uint16_t w,h,sz;
	UINT dmy, i;

	uint8_t* buffer;

	buffer=chHeapAlloc(NULL, 16384);

	s=f_open(&fil, fn, FA_READ);

	cx=3; cy=3;

	if (!buffer) {
		lcd_puts("Buffer allocation failed");
		return;
	}

	if (s==FR_NO_FILE) 	{
		lcd_puts("File does not exist.");
		return;
	}
	else if (s==FR_NOT_READY) {
		lcd_puts("Card not Inserted.");
		return;
	}

	f_read(&fil, buffer, 8, &dmy);

	s=buffer[0];
	if (s != 0x01) return;

	w=*((uint16_t*)&buffer[4]);
	h=*((uint16_t*)&buffer[6]);

	sz=504;

	lcd_setrect(x, x+w-1, y, y+h-1);
	lcd_locate(0,0);
	lcd_cmd(0x22);

	palClearPad(LCD_CS_GPIO, LCD_CS_PIN);

	do
	{
		spiAcquireBus(&SPID1);
		f_read(&fil, buffer, sz, &dmy);
		spiReleaseBus(&SPID1);

		for (i=0;i<dmy;i+=2)
		{
			uint16_t x=__REV16(*((uint16_t*)&buffer[i]));
			lcd_lld_write(x);
		}

		if (dmy<sz) break;

		sz=16384;
	} while (1);

	palSetPad(LCD_CS_GPIO, LCD_CS_PIN);

	lcd_setrect(0, 239, 0, 319);
	lcd_locate(0,0);

	chHeapFree(buffer);

	// Close file and return
	f_close(&fil);
}

#endif

// Test Code
#if 1
static void testEventHandler(uint8_t evt)
{
	if (evt && evt < 11)
	{
		lcd_cls();
		ui_displayPreviousMenu();
	}
}

// RGB Test Pattern 1
static void testRGB1(void)
{
	lcd_cls();
	lcd_filledrect(RED,0,0,240,110);
	lcd_filledrect(GREEN,0,110,240,110);
	lcd_filledrect(BLUE,0,220,240,100);
	ui_sethandler((pfn)testEventHandler);
}

// RGB Test Pattern 2
static void testRGB2(void)
{
	lcd_cls();
	lcd_filledrect(RED,0,0,30,320);
	lcd_filledrect(GREEN,30,0,30,320);
	lcd_filledrect(BLUE,60,0,30,320);
	lcd_filledrect(BLACK,90,0,30,320);
	lcd_filledrect(CYAN,120,0,30,320);
	lcd_filledrect(MAGENTA,150,0,30,320);
	lcd_filledrect(YELLOW,180,0,30,320);
	lcd_filledrect(WHITE,210,0,30,320);
	ui_sethandler((pfn)testEventHandler);
}

// RGB Test Pattern 3
static void testRGB3(void)
{
	lcd_cls();
	int i=0;
	for (int x=0;x<240;x+=20)
	{
		for (int y=0;y<320;y+=20)
		{
			lcd_filledrect(colors[i%8], x, y, 20, 20);
			i++;
		}
		i++;
	}
	ui_sethandler((pfn)testEventHandler);
}

static void testImages(void)
{
	if (playerThread) return;

	uint32_t k=halGetCounterValue();

	//display_image("newImage.raw", 0, 0);
	display_bitmap("newImage.bmp", 0, 0);

	k=halGetCounterValue()-k;

	chprintf((BaseChannel*)&SD2, "Time taken:%ld\r\n", k);

	ui_sethandler((pfn)testEventHandler);
}

#endif

// About Screen
#if 1
static void displayAboutScreen(void)
{
	ui_clearUserArea();

	ui_drawTitleBar("About Application");
	ui_drawBottomBar(NULL, "Touch To Exit", NULL);

	cx=3; cy=UI_TBARHEIGHT+3;

	font=font_Larger;
	lcd_puts("STM32F4Discovery GUI Demo\n");

	font=font_MediumBold;
	cy+=7;

	lcd_puts("A demo of a GUI library, drawing \n");
	lcd_puts("application and a wave player.\n");

	cy+=7;
	lcd_puts("Compiled On: "__DATE__", "__TIME__"\n");
	lcd_puts("Compiler: "CH_COMPILER_NAME"\n");
	lcd_puts("Platform: STM32F4DISCOVERY\n");
	xprintf("Device ID:%lX\n", *((uint32_t*)0xE0042000));

	cy+=7;
	lcd_puts("Powered by ChibiOS\n");
	lcd_puts("Kernel Version:"CH_KERNEL_VERSION);

	ui_sethandler((pfn)testEventHandler);
}
#endif

// Application Title
const char appTitle[]="STM32F4D-ChibiOS LCD GUI Demo";
Thread* mainThread;

// Display the title and bottom area
void displayStartUpScreen(void)
{
	ui_drawTitleBar(appTitle);
	ui_drawBottomBar(0, "Menu", "Music");

	cx=3; cy=UI_TBARHEIGHT+18;

	xprintf("Device ID:%lX\n", *((uint32_t*)0xE0042000));
}

// Display Main Screen (Clears the user area)
void displayMainScreen(void)
{
	ui_clearUserArea();

	displayStartUpScreen();
	ui_sethandler((pfn)mainEventHandler);
}

// Get Key Press
uint8_t getkey(void)
{
	static uint8_t state=0;

	uint8_t key=0;

	if (state) {
		if (tsc_isPenDown())
		{
			if (lcd_getpoint())
			{
				if (lcd_blstate==LCD_BL_OFF)
				{
					lcd_blup();
					return 0;
				}
				lcd_blup();

				if (py > 290 && py < 320)
				{
					if (px > 0 && px < 60)	key=BTN_LEFT;
					if (px > 80 && px < 180) key=BTN_MID_SEL;
					if (px > 200 && px < 240) key=BTN_RIGHT;
				}

				if (py > 200 && py < 290)
				{
					if (px > 0 && px < 60) key=BTN_MID_UP;
					if (px > 180 && px < 240) key=BTN_MID_DOWN;
				}
			}
		}
		state=0;
	}
	else if (tsc_isPenDown())
	{
		state=1;
	}


	return key;
}

void mainEventHandler(uint8_t evt)
{
	int8_t x, y, z;
	switch (evt)
	{
		case BTN_MID_SEL: 	ui_clearUserArea();
							ui_displayMenu(subMenu1,1);
							break;

		case 0xFF:

			x = (int8_t)lis302dlReadRegister(&SPID1, LIS302DL_OUTX);
			y = (int8_t)lis302dlReadRegister(&SPID1, LIS302DL_OUTY);
			z = (int8_t)lis302dlReadRegister(&SPID1, LIS302DL_OUTZ);

			lcd_gotoxy(3, 70);
			xprintf("X:%3d   \nY:%3d   \nZ:%3d   ",
							(int16_t)x,
							(int16_t)y,
							(int16_t)z);
			break;
	}
}

__attribute__  ((noreturn))
static msg_t Thread1(void *arg) {
	(void)arg;
	chRegSetThreadName("blinker");

	while (TRUE)
	{
		key=getkey();

		ui_executecmd(key);
		chThdSleep(50);
	}
}

// Hardware Initialization
static void hw_init(void)
{
	// Init Serial Port
	sdStart(&SD2, NULL);
	palSetPadMode(GPIOA, 2, PAL_MODE_ALTERNATE(7));
	palSetPadMode(GPIOA, 3, PAL_MODE_ALTERNATE(7));

	// Init ADC
//	adcStart(&ADCD1, NULL);
//	adcSTM32EnableTSVREFE();
//	palSetPadMode(GPIOC, 1, PAL_MODE_INPUT_ANALOG);

	// Init LIS302DL Accelerometer
	spiStart(&SPID1, &hs_spicfg);
	lis302dlWriteRegister(&SPID1, LIS302DL_CTRL_REG1, 0x43);
	lis302dlWriteRegister(&SPID1, LIS302DL_CTRL_REG2, 0x00);
	lis302dlWriteRegister(&SPID1, LIS302DL_CTRL_REG3, 0x00);

	// Init LCD & TSC (with LCD)
	lcd_initc();
	xdev_out(lcd_putchar);

	// Init MMC/SD Card
#ifdef USE_MMC_CARD
	palSetPadMode(GPIOC, 4, PAL_MODE_OUTPUT_PUSHPULL);
	palSetPad(GPIOC, 4);
	mmcObjectInit(&MMCD1, &SPID1,
				&ls_spicfg, &hs_spicfg,
				mmc_is_protected, mmc_is_inserted);
	mmcStart(&MMCD1, NULL);
#endif

	codec_hw_init();
}

static VirtualTimer vt1;

static void led_toggle(void *p)
{
	p;
	palTogglePad(GPIOD, GPIOD_LED3);
	chVTSetI(&vt1, 500, led_toggle, NULL);
}

// Entry Point
int main(void)
{
#ifdef USE_MMC_CARD
	// Event Handler Array
	static const evhandler_t evhndl[] = {
		InsertHandler,
		RemoveHandler
	  };
	struct EventListener el0, el1;
#endif

	halInit();
	chSysInit();

	mainThread=chThdSelf();

	hw_init();

#ifdef USE_MMC_CARD
	// Register the event handler
	chEvtRegister(&MMCD1.inserted_event, &el0, 0);
	chEvtRegister(&MMCD1.removed_event, &el1, 1);

	// Invoke event handlers for card insertion immediately
	chEvtDispatch(evhndl, chEvtWaitOne(ALL_EVENTS));
#endif

	chThdCreateFromHeap(NULL, 1024, NORMALPRIO, Thread1, NULL);

	lcd_gotoxy(3, 30);
	lcd_puts("Hello, world");

	displayMainScreen();

	chprintf((BaseChannel *)&SD2, "Hello World!\r\n");

	chVTSetI(&vt1, 500, led_toggle, NULL);

	while (TRUE) {
		ui_executecmd(0xFF);
		chThdSleep(100);
	}
}
