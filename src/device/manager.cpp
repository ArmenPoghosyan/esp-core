#include "device/manager.h"

#include <Arduino.h>

#include "device/device.h"
#include "env.h"

#include <utility>

DeviceManager& DeviceManager::instance() {
	static DeviceManager manager;
	return manager;
}

void DeviceManager::register_device(IDevice* device) {
	if (!device) {
		return;
	}

	devices[device->get_id()] = device;

	device->on_state_updated([this, device](AbilityType type, const StateValue& value)
		{
			notify_device_state_updated(*device, type, value);
		}
	);
}

void DeviceManager::unregister_device(IDevice* device) {
	if (!device) {
		return;
	}

	const auto it = devices.find(device->get_id());
	if (it != devices.end() && it->second == device) {
		devices.erase(it);
	}

	states.erase(device);
}

IDevice* DeviceManager::get_device(uint64_t id) const {
	const auto it = devices.find(id);
	return it == devices.end() ? nullptr : it->second;
}

const std::map<uint64_t, IDevice*>& DeviceManager::get_devices() const {
	return devices;
}

const std::map<AbilityType, StateValue>* DeviceManager::get_device_states(const IDevice* device) const {
	auto it = states.find(device);
	if (it == states.end()) {
		return nullptr;
	}

	return &it->second;
}

void DeviceManager::on_device_state_updated(std::function<void(const IDevice&, AbilityType, const StateValue&)> callback) {
	device_state_update_listeners.push_back(std::move(callback));
}

void DeviceManager::notify_device_state_updated(const IDevice& device, AbilityType type, const StateValue& value) {
	states[&device][type] = value;

	for (const auto& listener : device_state_update_listeners) {
		listener(device, type, value);
	}
}

String DeviceManager::get_mac_address() const {
	const uint64_t mac = ESP.getEfuseMac();
	char text[18];
	snprintf(text, sizeof(text), "%02X:%02X:%02X:%02X:%02X:%02X",
		static_cast<uint8_t>(mac), static_cast<uint8_t>(mac >> 8), static_cast<uint8_t>(mac >> 16),
		static_cast<uint8_t>(mac >> 24), static_cast<uint8_t>(mac >> 32), static_cast<uint8_t>(mac >> 40));
	return String(text);
}

String DeviceManager::get_ap_name() const {
	const uint64_t mac = ESP.getEfuseMac();
	char text[32];
	snprintf(text, sizeof(text), "%s-%02X%02X", WIFI_AP_PREFIX, static_cast<uint8_t>(mac >> 32), static_cast<uint8_t>(mac >> 40));
	return String(text);
}

const char* DeviceManager::get_board_name() const {
	#ifdef ARDUINO_BOARD
	return ARDUINO_BOARD;
	#else
	return "unknown";
	#endif
}

const char* DeviceManager::get_platform() const {
	#if defined(ESP32)
	return "ESP32";
	#elif defined(ESP8266)
	return "ESP8266";
	#else
	return "unknown";
	#endif
}

JsonDocument DeviceManager::to_json_document(uint8_t flags) const {
	JsonDocument doc;
	JsonObject json = doc.to<JsonObject>();

	if (flags & JSON_BOARD_INFO) {
		JsonObject info = json["info"].to<JsonObject>();
		info["name"] = DEVICE_NAME;
		info["board"] = get_board_name();
		info["hw_version"] = DEVICE_HARDWARE_VERSION;
		info["sw_version"] = DEVICE_SOFTWARE_VERSION;
		info["mac_address"] = get_mac_address();
	}

	if (flags & JSON_DEVICE_LIST) {
		JsonObject list = json["devices"].to<JsonObject>();
		for (const auto& entry : devices) {
			char key[DEVICE_ID_LENGTH + 1];
			snprintf(key, sizeof(key), "%llu", static_cast<unsigned long long>(entry.first));
			entry.second->to_json(list[key].to<JsonObject>(), flags);
		}
	}

	return doc;
}

String DeviceManager::to_json(uint8_t flags) const {
	String text;
	serializeJson(to_json_document(flags), text);
	return text;
}

DeviceManager& device_manager = DeviceManager::instance();
