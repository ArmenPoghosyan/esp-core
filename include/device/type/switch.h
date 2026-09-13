#pragma once

#include "device/device.h"

/**
 * @brief A class representing a switch device.
 * This class is a specialization of GenericDevice for devices of type SWITCH.
 */
template<typename... Abilities>
class Switch : public GenericDevice<Abilities...> {
	public:
	Switch(const char* name) : GenericDevice<Abilities...>(DeviceType::SWITCH, name) {}
};
