#pragma once

#include <Arduino.h>

#include <freertos/timers.h>

class LED {
	private:
	int pin;
	TimerHandle_t timer;
	bool state = false;

	public:
	LED(int pin = LED_BUILTIN, int MODE = OUTPUT);

	void on();
	void off();
	void toggle();
	void set(bool state);
	bool isOn();
	void blink(unsigned long interval = 500);
	void stop_blink();
};
