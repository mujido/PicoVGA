#pragma once

#include <bitset>
#include <initializer_list>

// How many checks before button press is considered stable.
constexpr unsigned BUTTON_CHECK_COUNT = 10;

// Interval (in microseconds) to check button state for debounce.
constexpr int BUTTON_CHECK_INTERVAL_US = 2'000;

// GPIO to use for next pattern button.
constexpr unsigned BUTTON_NEXT = 26;

// GPIO to use for previous pattern button.
constexpr unsigned BUTTON_PREV = 27;

using ButtonState = std::bitset<32>;

// Get the current debounced state of all buttons.
ButtonState GetDebouncedButtonState();

// Initialize button functionality.
void InitButtons(const std::initializer_list<unsigned>& buttons);
