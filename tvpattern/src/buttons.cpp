#include "buttons.h"
#include <array>
#include <pico/time.h>
#include <hardware/gpio.h>

namespace
{
    std::array<uint32_t, BUTTON_CHECK_COUNT> buttonStates = {0};
    unsigned buttonStatesIndex = 0;

    ButtonState usedButtonsMask;

    bool KeyDebounceTimerCallback(repeating_timer_t*)
    {
        uint32_t allButtons = gpio_get_all();
        buttonStates[buttonStatesIndex] = allButtons;
        buttonStatesIndex++;

        if (buttonStatesIndex >= buttonStates.size())
            buttonStatesIndex = 0;

        return true;
    }
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

	return debounced & usedButtonsMask;
}

void InitButtons(const std::initializer_list<unsigned>& buttons)
{
	for (auto btn : buttons)
	{
        usedButtonsMask.set(btn);

		gpio_init(btn);
		gpio_set_dir(btn, false);
		gpio_pull_up(btn);
	}

	static repeating_timer_t debounce_timer;
	add_repeating_timer_us(-BUTTON_CHECK_INTERVAL_US, KeyDebounceTimerCallback, nullptr, &debounce_timer);
}
