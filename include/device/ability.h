#pragma once

#include "device/device.h"

class AbilityBase {
	private:
	IDevice* device;
	AbilityType type;

	public:
	AbilityBase(IDevice* device, AbilityType type);

	protected:

	virtual bool get_state() const = 0;
};

template<AbilityType T>
class Ability : public AbilityBase {
	public:
	Ability(IDevice* device) : AbilityBase(device, T) {}
};
