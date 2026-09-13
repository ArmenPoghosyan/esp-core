#include "device/device.h"

#include "device/ability.h"
#include "device/manager.h"

#include <utility>

IDevice::IDevice(DeviceType type, const char* name) : type(type), name(name) {
	DeviceManager::instance().register_device(this);
}

IDevice::~IDevice() {
	DeviceManager::instance().unregister_device(this);
}

DeviceType IDevice::get_type() const {
	return type;
}

const char* IDevice::get_name() const {
	return name;
}

void IDevice::register_ability(AbilityBase* ability) {
	if (!ability) {
		return;
	}

	abilities.push_back(ability);
	// DeviceManager::instance().notify_device_state_updated(*this, ability->get_type(), ability->get_state_value());
}

const std::vector<AbilityBase*>& IDevice::get_abilities() const {
	return abilities;
}

std::vector<AbilityState> IDevice::get_ability_states() const {
	std::vector<AbilityState> result;
	result.reserve(abilities.size());

	for (const AbilityBase* ability : abilities) {
		result.push_back(AbilityState{ability->get_type(), ability->get_state_value()});
	}

	return result;
}

void IDevice::on_state_updated(std::function<void(AbilityType, const StateValue&)> callback) {
	state_update_listeners.push_back(std::move(callback));
}

void IDevice::notify_state_updated(AbilityType type, const StateValue& value) {
	for (const auto& callback : state_update_listeners) {
		callback(type, value);
	}
}

