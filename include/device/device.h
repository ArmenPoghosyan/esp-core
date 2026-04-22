#pragma once

#include "device/consts.h"

class IDevice {
	private:
	DeviceType type;
	const char* name;

	public:
	IDevice(DeviceType type, const char* name);

	/**
	 * @brief Get the type of the device
	 */
	DeviceType get_type();

	/**
	 * @brief Get the name of the device
	 */
	const char* get_name();
};

template<typename... Abilities>
class GenericDevice : public IDevice, public Abilities... {
	public:
	GenericDevice(DeviceType type, const char* name) : IDevice(type, name), Abilities(this)... {
		//
	}
};
