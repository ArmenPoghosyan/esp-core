#pragma once

#include "device/device.h"
#include "device/ability/on_off.h"
#include "device/ability/brightness.h"
#include "device/ability/color.h"
#include "led/led.h"

/**
 * @brief A class representing a light device.
 * This class is a specialization of GenericDevice for devices of type LIGHT.
 */
template<typename... Abilities>
class Light : public GenericDevice<Abilities...> {
	public:
	Light(const char* name) : GenericDevice<Abilities...>(DeviceType::LIGHT, name) {}

	/**
	 * @brief Attaches the light device to a physical LED.
	 * This allows the light device to control the LED based on its state updates.
	 */
	void attach_to_led(LED& led) {
		this->led = &led;

		this->on_state_updated(
			[this](AbilityType type, const StateValue& value) {
				if (!this->led) return;
				switch (type) {
					case AbilityType::ON_OFF: {
						bool is_on = convert_state<bool>(value);
						this->led->set(is_on);
					} break;

					case AbilityType::BRIGHTNESS: {
						uint8_t brightness = convert_state<uint8_t>(value);
						this->led->set_brightness(brightness);
					} break;

					default: break;
				}
			}
		);
	}

	private:
	LED* led = nullptr;
};

typedef Light<OnOff> SimpleLight;
typedef Light<OnOff, Brightness> DimmableLight;
typedef Light<OnOff, Brightness, Color> ColorDimmableLight;
typedef Light<OnOff, Color> ColorLight;
