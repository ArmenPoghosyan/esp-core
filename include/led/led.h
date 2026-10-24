#pragma once

#include <Arduino.h>

#include <freertos/timers.h>

#include <functional>
#include <vector>

class LED {
	private:
	int pin;
	TimerHandle_t timer = nullptr;
	bool state = false;
	uint8_t brightness = 255;
	std::vector<std::function<void(bool)>> change_callbacks;

	void fire_change();   // runs the on_change callbacks with the current state

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
	void set_brightness(uint8_t brightness);
	void set_brightness_percentage(uint8_t brightness);
	uint8_t get_brightness() const;
	uint8_t get_brightness_percentage() const;
	void on_change(std::function<void(bool)> callback);   // on/off state changed; argument is the new state
};
