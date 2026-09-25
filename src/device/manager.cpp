#include "device/manager.h"

#include <Arduino.h>

#include "device/device.h"

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

DeviceManager device_manager;
