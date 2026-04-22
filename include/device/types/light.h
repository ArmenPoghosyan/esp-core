#pragma once

#include "device/device.h"
#include "device/abilities/on_off.h"

/**
 * @brief A class representing a light device.
 * This class is a specialization of GenericDevice for devices of type LIGHT.
 */
template<typename... Abilities>
class Light : public GenericDevice<Abilities...> {
	public:
	Light(const char* name) : GenericDevice<Abilities...>(DeviceType::LIGHT, name) {}
};

/**
 * @brief A basic light device that supports on/off functionality.
 */
class BasicLight : public Light<OnOff> {
	public:
	BasicLight(const char* name) : Light<OnOff>(name) {}
};

