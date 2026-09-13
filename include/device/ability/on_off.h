#pragma once

#include "device/ability.h"

class OnOff : public Ability<AbilityType::ON_OFF, bool> {
	public:
	explicit OnOff(IDevice* device) : Ability(device, false) {
		// On/Off defaults to off.
	}

	/**
	 * @brief Turns the device on.
	 */
	void turn_on() {
		set_state(true);
	}

	/**
	 * @brief Turns the device off.
	 */
	void turn_off() {
		set_state(false);
	}

	/**
	 * @brief Checks if the device is currently on.
	 * @return true if the device is on, false otherwise.
	 */
	bool is_on() const {
		return get_state();
	}

	/**
	 * @brief Toggles the device state between on and off.
	 */
	void toggle_on_off() {
		set_state(!get_state());
	}
};
