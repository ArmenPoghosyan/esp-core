#include "led/led.h"

LED::LED(int pin, int MODE) {
	pinMode(pin, MODE);
	this->pin = pin;
}

LED::~LED() {
	stop_blink();
}

void LED::on() {
	analogWrite(pin, brightness);   // PWM duty = brightness (255 = full on)
	if (!state) {
		state = true;
		fire_change();
	}
}

void LED::off() {
	analogWrite(pin, 0);
	if (state) {
		state = false;
		fire_change();
	}
}

void LED::toggle() {
	if (state) {
		off();
	} else {
		on();
	}
}

void LED::set(bool state) {
	if (state) {
		on();
	} else {
		off();
	}
}

bool LED::isOn() {
	return state;
}

void LED::set_brightness(uint8_t value) {
	brightness = value;
	if (state) {
		analogWrite(pin, brightness);
	}
}

void LED::set_brightness_percentage(uint8_t value) {
	set_brightness(map(value, 0, 100, 0, 255));
}

uint8_t LED::get_brightness() const {
	return brightness;
}

uint8_t LED::get_brightness_percentage() const {
	return map(brightness, 0, 255, 0, 100);
}

void LED::on_change(std::function<void(bool)> callback) {
	change_callbacks.push_back(std::move(callback));
}

// Called after the state has changed, so a callback sees the new state.
void LED::fire_change() {
	for (const auto& callback : change_callbacks) {
		callback(state);
	}
}

void LED::blink(unsigned long interval) {
	stop_blink();

	timer = xTimerCreate("LED Blink Timer", pdMS_TO_TICKS(interval), pdTRUE, (void*)this, [](TimerHandle_t xTimer)
		{
			LED* led = (LED*)pvTimerGetTimerID(xTimer);
			if (led) {
				led->toggle();
			}
		}
	);

	xTimerStart(timer, 0);
}

void LED::stop_blink() {
	if (timer) {
		xTimerDelete(timer, 0);
		timer = nullptr;
	}
}
