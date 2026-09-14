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
	state = true;
}

void LED::off() {
	analogWrite(pin, 0);
	state = false;
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
		analogWrite(pin, brightness);   // apply immediately if currently on
	}
}

uint8_t LED::get_brightness() {
	return brightness;
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
