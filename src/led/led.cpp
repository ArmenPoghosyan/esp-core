#include "led/led.h"

LED::LED(int pin, int MODE) {
	pinMode(pin, MODE);
	this->pin = pin;
}

LED::~LED() {
	stop_blink();
}

void LED::on() {
	digitalWrite(pin, HIGH);
	state = true;
}

void LED::off() {
	digitalWrite(pin, LOW);
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
