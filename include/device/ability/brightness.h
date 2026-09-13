#pragma once

#include "device/ability.h"

class Brightness : public Ability<AbilityType::BRIGHTNESS, uint8_t> {
	public:
	explicit Brightness(IDevice* device) : Ability(device, 100) {
		// Brightness defaults to 100%.
	}

	uint8_t get_brightness() const {
		return get_state();
	}

	void set_brightness(uint8_t value) {
		if (value > 100) {
			value = 100;
		}

		set_state(value);
	}
};
