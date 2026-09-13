#pragma once

#include <Arduino.h>

#include <freertos/timers.h>

class LED {
	private:
	int pin;
	TimerHandle_t timer = nullptr;
	bool state = false;

	public:
	LED(int pin = LED_BUILTIN, int MODE = OUTPUT);
	~LED();

	// LED owns a FreeRTOS timer handle; copying would double-free it.
	LED(const LED&) = delete;
	LED& operator=(const LED&) = delete;

	void on();
	void off();
	void toggle();
	void set(bool state);
	bool isOn();
	void blink(unsigned long interval = 500);
	void stop_blink();
};
