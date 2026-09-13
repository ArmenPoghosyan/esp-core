#pragma once

#include "device/device.h"

/**
 * @brief A class representing a light device.
 * This class is a specialization of GenericDevice for devices of type LIGHT.
 */
template<typename... Abilities>
class Light : public GenericDevice<Abilities...> {
	public:
	Light(const char* name) : GenericDevice<Abilities...>(DeviceType::LIGHT, name) {}
};
