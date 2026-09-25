#pragma once

#include "device/device.h"
#include "device/ability/on_off.h"

/**
 * @brief A class representing a switch device.
 * This class is a specialization of GenericDevice for devices of type SWITCH.
 */
template<typename... Abilities>
class Switch : public GenericDevice<Abilities...> {
	public:
	Switch(const char* name) : GenericDevice<Abilities...>(DeviceType::SWITCH, name) {}
};

typedef Switch<OnOff> SimpleSwitch;
