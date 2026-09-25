#include "device/device.h"

#include <Arduino.h>

#include "device/ability.h"
#include "device/manager.h"

#include <utility>

namespace {
	// An id has exactly 18 digits, so it never starts with 0: [ID_MIN, 10 * ID_MIN).
	constexpr uint64_t ID_MIN = 100000000000000000ULL;
	constexpr uint64_t ID_COUNT = 17 * ID_MIN;   // how many distinct ids exist
}

IDevice::IDevice(DeviceType type, const char* name) : type(type), name(name) {
	id = generate_id();
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

uint64_t IDevice::get_id() const {
	return id;
}

uint64_t IDevice::generate_id() const {
	// FNV-1a over the MAC bytes and then the name. The MAC is the factory one
	// (eFuse), so it never changes and no two chips share it.
	const uint64_t mac = ESP.getEfuseMac();
	uint64_t hash = 14695981039346656037ULL;
	const auto mix = [&hash](uint8_t byte) { hash = (hash ^ byte) * 1099511628211ULL; };

	for (int i = 0; i < 6; i++) {
		mix(static_cast<uint8_t>(mac >> (8 * i)));
	}

	for (const char* c = name; c && *c; c++) {
		mix(static_cast<uint8_t>(*c));
	}

	return ID_MIN + hash % ID_COUNT;
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
		result.push_back(AbilityState{ ability->get_type(), ability->get_state_value() });
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

