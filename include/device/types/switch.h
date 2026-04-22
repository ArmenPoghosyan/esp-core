#pragma once

#include "device/device.h"
#include "device/abilities/on_off.h"

/**
 * @brief A class representing a switch device.
 * This class is a specialization of GenericDevice for devices of type SWITCH.
 */
template<typename... Abilities>
class Switch : public GenericDevice<Abilities...> {
	public:
	Switch(const char* name) : GenericDevice<Abilities...>(DeviceType::SWITCH, name) {}
};

/**
 * @brief A basic switch device that supports on/off functionality.
 */
class BasicSwitch : public Switch<OnOff> {
	public:
	BasicSwitch(const char* name) : Switch<OnOff>(name) {}
};
