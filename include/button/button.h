#pragma once

#include <Arduino.h>

#include <freertos/timers.h>

#include <functional>
#include <vector>

class Button {
	private:
	struct LongPressListener {
		unsigned long duration_ms;
		std::function<void()> callback;
		bool fired;
	};

	uint8_t pin;
	uint8_t mode;
	uint8_t pressed_state;

	unsigned long debounce_ms;
	unsigned long poll_interval_ms;
	unsigned long last_raw_change_ms;
	unsigned long press_start_ms;

	uint8_t raw_state;
	uint8_t stable_state;
	bool is_pressed;
	bool timer_started;
	TimerHandle_t timer;

	std::function<void()> press_callback;
	std::vector<LongPressListener> long_press_listeners;

	void process();
	void ensure_timer_started();
	static void timer_tick(TimerHandle_t timer);

	public:
	explicit Button(uint8_t pin, uint8_t mode = INPUT_PULLUP, unsigned long debounce_ms = 30, unsigned long poll_interval_ms = 10);
	~Button();

	void start();
	void stop();

	void onPress(std::function<void()> callback);
	void onLongPress(unsigned long duration_ms, std::function<void()> callback);

	void setDebounce(unsigned long debounce_ms);
	bool pressed() const;
};
