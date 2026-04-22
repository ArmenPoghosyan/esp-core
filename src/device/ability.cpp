#include "device/ability.h"

AbilityBase::AbilityBase(IDevice* device, AbilityType type) : device(device), type(type) {
	if (this->device) {
		this->device->register_ability(this);
	}
}

AbilityType AbilityBase::get_type() const {
	return type;
}

IDevice* AbilityBase::get_device() const {
	return device;
}

void AbilityBase::notify_state_changed(const StateValue& value) {
	if (!device) {
		return;
	}

	device->notify_state_updated(type, value);
}
