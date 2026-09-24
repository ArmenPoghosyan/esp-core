#pragma once

#include "device/ability.h"

class Color : public Ability<AbilityType::COLOR, uint32_t> {
	public:
	explicit Color(IDevice* device) : Ability(device, 0x00FFFFFF, "color") {
		// Default color is white in RGB packed format.
	}

	uint32_t get_color() const {
		return get_state();
	}

	void set_color(uint32_t color) {
		set_state(color);
	}
};
