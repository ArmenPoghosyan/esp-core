#pragma once

#include "device/ability.h"

class Brightness : public Ability<AbilityType::BRIGHTNESS, int> {
	public:
	explicit Brightness(IDevice* device) : Ability(device, 0, "brightness") {
		// Brightness defaults to 0%.
	}

	int get_brightness() const {
		return get_state();
	}

	void set_brightness(int value) {
		if (value < 0) {
			value = 0;
		}

		if (value > 100) {
			value = 100;
		}

		set_state(value);
	}
};
