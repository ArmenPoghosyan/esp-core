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

		const auto apply_to_led = [this](AbilityType type, const StateValue& value)
			{
				if (!this->led) return;
				switch (type) {
					case AbilityType::ON_OFF:
						this->led->set(convert_state<bool>(value));
						break;

					case AbilityType::BRIGHTNESS:
						this->led->set_brightness_percentage(convert_state<uint8_t>(value));
						break;

					default: break;
				}
			}
			//
		;

		for (const AbilityState& state : this->get_ability_states()) {
			apply_to_led(state.type, state.value);
		}

		this->on_state_updated(apply_to_led);
	}

	private:
	LED* led = nullptr;
};

typedef Light<OnOff> SimpleLight;
typedef Light<OnOff, Brightness> DimmableLight;
typedef Light<OnOff, Brightness, Color> ColorDimmableLight;
typedef Light<OnOff, Color> ColorLight;
