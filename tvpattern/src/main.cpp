
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

#define PI_MONOSCOPE 31

#include "include.h"
#include <pico/time.h>
#include <hardware/gpio.h>
#include <array>
#include <bitset>
#include <initializer_list>

u16 Rows[962];	// RLE rows
u8 Img[180000] __attribute__ ((aligned(4))); // RLE image
int MonoSel = 22; // selected videomode

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
	MonoSel = inx;
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

// display list of available videomodes
void MonoList()
{
	printf("\n");

	// print header
	printf(" n | key | timing | Hres | Vres |ratio|  scan | FPS | sysclk\n");
	printf("---+-----+--------+------+------+-----+-------+-----+-------\n");

	// print modes
	int i;
	sMono* mono;
	for (i = 0; i < MONO_NUM; i++)
	{
		mono = &Mono[i];

		// prepare videomode configuration
		MonoCfg(i);

		// index
		printf("%2u |", i);

		// key
		if (i < 10)
			printf("  %c  |", i + '0');
		else
			printf("  %c  |", i - 10 + 'A');

		// timing name
		if (i == PI_MONOSCOPE)
			printf(" PICO   |");
		else
			printf(" %s  |", Vmode.name);

		// horizontal resolution
		int w = Vmode.width;
		printf("%5u |", w);

		// vertical resolution
		int h = Vmode.height;
		printf("%5u |", h);

		// ratio
#define EPS 0.02f
		float r = (float)w/h;
		if (fabs(r - 4.0/3) < EPS)
			printf(" 4:3 |");
		else if (fabs(r - 5.0/4) < EPS)
			printf(" 5:4 |");
		else if (fabs(r - 3.0/2) < EPS)
			printf(" 3:2 |");
		else if (fabs(r - 10.0/9) < EPS)
			printf("10:9 |");
		else if (fabs(r - 16.0/9) < EPS)
			printf("16:9 |");
		else if (fabs(r - 8.0/5) < EPS)
			printf("16:10|");
		else
			printf("     |");

		// horizontal frequency
		float f = Vmode.freq*1000.0f; // system frequency at Hz
		float k = f/Vmode.div; // frequency of state machine clock at Hz
		int hor = (int)(k/Vmode.htot + 0.5f); // horizontal frequency
		printf("%6u |", hor);

		// vertical frequency
		int vert = (int)(k/Vmode.htot/Vmode.vtot + 0.5f); // vertical frequency
		printf("%4u |", vert);

		// system frequency 
		printf("%7u", Vmode.freq);

		// current videomode
		if (i == MonoSel) printf(" *");

		// end of line
		printf("\n");
	}

	// print current frequency
	uint hz = clock_get_hz(clk_sys);
	printf("current mode: %u, current sysclk: %u kHz\n", MonoSel, hz/1000);
	printf("Press key: ");
}

// constexpr unsigned BUTTON_CHECK_INTERVAL_MS = 5;
// constexpr unsigned BUTTON_PRESS_DURATION_MS = 10;
// constexpr unsigned BUTTON_RELEASE_DURATION_MS = 50;
// constexpr unsigned BUTTON_CHECK_COUNT = std::max(BUTTON_PRESS_DURATION_MS, BUTTON_RELEASE_DURATION_MS) / BUTTON_CHECK_INTERVAL_MS;
constexpr unsigned BUTTON_CHECK_COUNT = 10;
constexpr unsigned BUTTON_NEXT = 26;
constexpr unsigned BUTTON_PREV = 27;

using ButtonState = std::bitset<32>;

static std::array<uint32_t, BUTTON_CHECK_COUNT> buttonStates = {0};
static unsigned buttonStatesIndex = 0;

bool KeyDebounceTimerCallback(repeating_timer_t*)
{
	uint32_t allButtons = gpio_get_all();
	buttonStates[buttonStatesIndex] = allButtons;
	buttonStatesIndex++;

	if (buttonStatesIndex >= buttonStates.size())
		buttonStatesIndex = 0;

	return true;
}

ButtonState GetDebouncedButtonState()
{
	ButtonState debounced;
	debounced.set();

	for (auto val : buttonStates)
	{
		// Invert state because buttons are active-low
		debounced &= ~val;
	}

	return debounced;	
}

constexpr ButtonState MakeButtonMask(const std::initializer_list<unsigned>& buttons)
{
	unsigned mask = 0;
	
	for (auto btn : buttons)
		mask |= 1 << btn;

	return mask;
}

void InitButtons()
{
	for (auto btn : { BUTTON_NEXT, BUTTON_PREV })
	{
		gpio_init(btn);
		gpio_set_dir(btn, false);
		gpio_pull_up(btn);
	}

	static repeating_timer_t debounce_timer;
	add_repeating_timer_us(-2'000, KeyDebounceTimerCallback, nullptr, &debounce_timer);
}

int main()
{
	constexpr auto buttonMask = MakeButtonMask({BUTTON_NEXT, BUTTON_PREV});

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

	InitButtons();

	// Display starting image
	unsigned imageIdx = 0;
	DisplayImage(images[imageIdx], Mono[modeIndex]);

	ButtonState prevButtonState;

	while (true)
	{
		auto newButtonState = GetDebouncedButtonState();
		newButtonState &= buttonMask;

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
		__wfe();
	}
}
