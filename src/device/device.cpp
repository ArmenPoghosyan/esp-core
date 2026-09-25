#include "device/device.h"

#include <Arduino.h>

#include "device/ability.h"
#include "device/manager.h"

#include <type_traits>
#include <utility>

namespace {
	// An id has exactly 18 digits, so it never starts with 0: [ID_MIN, 10 * ID_MIN).
	constexpr uint64_t ID_MIN = 10000000000000000ULL;
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

namespace {
	// A StateValue into a JSON value: null when the ability has no value.
	void state_to_json(JsonVariant json, const StateValue& value) {
		std::visit([&json](const auto& v) {
			if constexpr (std::is_same_v<std::decay_t<decltype(v)>, std::monostate>) {
				json.clear();
			} else {
				json.set(v);
			}
			}, value);
	}
}

void IDevice::to_json(JsonObject json, uint8_t flags) const {
	if (flags & JSON_DEVICE_INFO) {
		// json["id"] = id;
		json["name"] = name;
		json["type"] = static_cast<uint8_t>(type);
	}

	if (flags & JSON_DEVICE_ABILITIES) {
		JsonArray list = json["abilities"].to<JsonArray>();
		for (const AbilityBase* ability : abilities) {
			list.add(static_cast<uint8_t>(ability->get_type()));
		}
	}

	if (flags & JSON_DEVICE_STATE) {
		JsonObject state = json["state"].to<JsonObject>();
		for (const AbilityBase* ability : abilities) {
			state_to_json(state[ability->get_name()].to<JsonVariant>(), ability->get_state_value());
		}
	}
}

String IDevice::to_json(uint8_t flags) const {
	JsonDocument doc;
	to_json(doc.to<JsonObject>(), flags);

	String text;
	serializeJson(doc, text);
	return text;
}

bool IDevice::from_json(JsonObjectConst state) {
	bool all_applied = true;

	for (JsonPairConst item : state) {
		// Only an ability this device has may be changed.
		AbilityBase* target = nullptr;
		for (AbilityBase* ability : abilities) {
			if (strcmp(ability->get_name(), item.key().c_str()) == 0) {
				target = ability;
				break;
			}
		}

		if (!target) {
			log_w("'%s' has no ability '%s'", name, item.key().c_str());
			all_applied = false;
		} else if (!target->set_state_from_json(item.value())) {
			log_w("'%s': bad value for '%s'", name, item.key().c_str());
			all_applied = false;
		}
	}

	return all_applied;
}

bool IDevice::from_json(const String& state) {
	JsonDocument doc;
	if (deserializeJson(doc, state) != DeserializationError::Ok || !doc.is<JsonObjectConst>()) {
		log_w("'%s': state is not a JSON object", name);
		return false;
	}

	return from_json(doc.as<JsonObjectConst>());
}

