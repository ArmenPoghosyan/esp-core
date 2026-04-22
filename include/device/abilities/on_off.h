#pragma once

#include "device/ability.h"

using Parent = Ability<AbilityType::ON_OFF>;

class OnOff : public Parent {
	using Parent::Ability;

	/**
	 * @brief Turns the device on.
	 */
	void turn_on();

	/**
	 * @brief Turns the device off.
	 */
	void turn_off();

	/**
	 * @brief Checks if the device is currently on.
	 * @return true if the device is on, false otherwise.
	 */
	bool is_on() const;

	/**
	 * @brief Toggles the device state between on and off.
	 */
	void toggle_on_off();
};
