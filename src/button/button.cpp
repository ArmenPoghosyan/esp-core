#include "button/button.h"

#include <utility>

Button::Button(uint8_t pin, uint8_t mode, unsigned long debounce_ms, unsigned long poll_interval_ms)
	: pin(pin), mode(mode), pressed_state(mode == INPUT_PULLUP ? LOW : HIGH), debounce_ms(debounce_ms),
	  poll_interval_ms(poll_interval_ms), last_raw_change_ms(0), press_start_ms(0), raw_state(HIGH), stable_state(HIGH),
	  is_pressed(false), timer_started(false), timer(nullptr) {
	pinMode(pin, mode);

	raw_state = digitalRead(pin);
	stable_state = raw_state;
	is_pressed = stable_state == pressed_state;

	if (is_pressed) {
		press_start_ms = millis();
	}
}

Button::~Button() {
	stop();
}

void Button::timer_tick(TimerHandle_t timer) {
	Button* button = static_cast<Button*>(pvTimerGetTimerID(timer));
	if (button) {
		button->process();
	}
}

void Button::ensure_timer_started() {
	if (!timer) {
		timer = xTimerCreate("ButtonTimer", pdMS_TO_TICKS(poll_interval_ms), pdTRUE, this, Button::timer_tick);
	}

	if (timer && !timer_started) {
		timer_started = xTimerStart(timer, 0) == pdPASS;
	}
}

void Button::start() {
	ensure_timer_started();
}

void Button::stop() {
	if (!timer) {
		return;
	}

	xTimerStop(timer, 0);
	xTimerDelete(timer, 0);
	timer = nullptr;
	timer_started = false;
}

void Button::process() {
	const unsigned long now = millis();
	const uint8_t current_raw = digitalRead(pin);

	if (current_raw != raw_state) {
		raw_state = current_raw;
		last_raw_change_ms = now;
	}

	if ((now - last_raw_change_ms) >= debounce_ms && stable_state != raw_state) {
		stable_state = raw_state;

		if (stable_state == pressed_state) {
			is_pressed = true;
			press_start_ms = now;

			for (auto& listener : long_press_listeners) {
				listener.fired = false;
			}

			if (press_callback) {
				press_callback();
			}
		} else {
			is_pressed = false;
		}
	}

	if (!is_pressed) {
		return;
	}

	const unsigned long held_ms = now - press_start_ms;
	for (auto& listener : long_press_listeners) {
		if (!listener.fired && held_ms >= listener.duration_ms) {
			listener.fired = true;
			if (listener.callback) {
				listener.callback();
			}
		}
	}
}

void Button::onPress(std::function<void()> callback) {
	press_callback = std::move(callback);
	ensure_timer_started();
}

void Button::onLongPress(unsigned long duration_ms, std::function<void()> callback) {
	long_press_listeners.push_back(LongPressListener{duration_ms, std::move(callback), false});
	ensure_timer_started();
}

void Button::setDebounce(unsigned long debounce_ms) {
	this->debounce_ms = debounce_ms;
}

bool Button::pressed() const {
	return is_pressed;
}
