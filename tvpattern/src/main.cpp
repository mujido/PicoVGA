
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************
// Set videomodes from USB console and display monoscope.
// To use UART console, edit "stdio output" in Makefile.

/*
 n | timing | Hres | Vres |ratio|  scan | FPS | sysclk
---+--------+------+------+-----+-------+-----+-------
00 | NTSCp  |  320 |  240 | 4:3 | 15734 |  60 | 122666
01 | VGA    |  320 |  240 | 4:3 | 31469 |  60 | 126000
*/

#include "include.h"
#include "buttons.h"
#include <initializer_list>

u16 Rows[962];	// RLE rows
u8 Img[180000] __attribute__ ((aligned(4))); // RLE image

// monoscope descriptor
typedef struct {
	int		width;		// width
	int		height;		// height
	int		full;		// full width
	const sVideo*	video;		// videomode timings
	Bool		dbly;		// double y
} sMono;

// available videomodes
sMono Mono[] = {
	{ 320, 240, 320, &VideoNTSCp, False }, // 4:3
	{ 320, 240, 320, &VideoVGA,   True }, // 4:3
};

#define MONO_NUM count_of(Mono) // number of videomodes

struct ImageInfo
{
	int		imgsize;	// image size
	const u16*	rows;		// pointer to rows
	const u8*	img;		// pointer to image
};

ImageInfo images[] = {
	{ count_of(Pattern1), Pattern1_rows, Pattern1 },
	{ count_of(Pattern2), Pattern2_rows, Pattern2 },
	{ count_of(Pattern3), Pattern3_rows, Pattern3 },
	{ count_of(Pattern4), Pattern4_rows, Pattern4 },
};

void DisplayImage(const ImageInfo& image, const sMono& mono)
{
	LayerOff(IMG_LAYER);

	// copy image into RAM buffer (flash is not fast enough)
	memcpy(Rows, image.rows, (mono.height+1)*2);
	memcpy(Img, image.img, image.imgsize);

	// setup layer 1 with RLE image of the monoscope
	LayerSetup(IMG_LAYER, Img, &Vmode, mono.width, mono.height, 0, Rows);
	LayerOn(IMG_LAYER);
}

// prepare videomode configuration
void MonoCfg(int inx)
{
	sMono* mono = &Mono[inx]; // selected videomode
	VgaCfgDef(&Cfg); // get default configuration
	Cfg.video = mono->video; // video timings
	Cfg.width = mono->width; // screen width
	Cfg.height = mono->height; // screen height
	Cfg.wfull = mono->full; // full width
	Cfg.dbly = mono->dbly; // double Y
	Cfg.mode[IMG_LAYER] = LAYERMODE_RLE; // layer mode
	VgaCfg(&Cfg, &Vmode); // calculate videomode setup
}

// init videomode
void MonoInit(int inx)
{
	// terminate current driver
	VgaInitReq(NULL);

	// prepare videomode configuration
	MonoCfg(inx);

	// initialize base layer 0 to simple color (will be not visible)
	ScreenClear(pScreen);
	sStrip* t = ScreenAddStrip(pScreen, Vmode.height);
	sSegm* g = ScreenAddSegm(t, Vmode.width);
	ScreenSegmColor(g, 0xff00ff00, 0x00ff00ff);

	// initialize system clock
	if (clock_get_hz(clk_sys) != Vmode.freq*1000)
		set_sys_clock_pll(Vmode.vco*1000, Vmode.pd1, Vmode.pd2);

	// reinitialize stdio UART
#if PICO_STDIO_UART
	setup_default_uart();
#endif

	// initialize videomode
	VgaInitReq(&Vmode);
}

int main()
{
	// run VGA core
	multicore_launch_core1(VgaCore);

	// run default video mode VGA 640x480
	int modeIndex = DEFAULT_MODE_INDEX;
	MonoInit(modeIndex);

	// initialize stdio
	stdio_init_all();

	for (auto led : {16, 17})
	{
		gpio_init(led);
		gpio_set_dir(led, true);
		gpio_put(led, false);
	}

	InitButtons({BUTTON_PREV, BUTTON_NEXT});

	// Display starting image
	unsigned imageIdx = 0;
	DisplayImage(images[imageIdx], Mono[modeIndex]);

	ButtonState prevButtonState;

	while (true)
	{
		auto newButtonState = GetDebouncedButtonState();
		auto changed = newButtonState ^ prevButtonState;

		if (changed.test(BUTTON_NEXT))
		{
			if (newButtonState.test(BUTTON_NEXT))
			{
				++imageIdx;
				if (imageIdx >= count_of(images))
					imageIdx = 0;

				DisplayImage(images[imageIdx], Mono[modeIndex]);
				gpio_put(16, true);
			}
			else
				gpio_put(16, false);
		}

		if (changed.test(BUTTON_PREV))
		{
			if (newButtonState.test(BUTTON_PREV))
			{
				if (imageIdx > 0)
					imageIdx--;
				else
					imageIdx = count_of(images) - 1;

				DisplayImage(images[imageIdx], Mono[modeIndex]);
				gpio_put(17, true);
			}
			else
				gpio_put(17, false);
		}

		prevButtonState = newButtonState;

		// Pause core until an event/interrupt occurs.
		__wfe();
	}
}
