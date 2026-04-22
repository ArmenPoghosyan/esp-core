#pragma once

#include "device/consts.h"

#include <functional>
#include <map>
#include <vector>

class IDevice;

class DeviceManager {
	private:
	std::vector<IDevice*> devices;
	std::map<const IDevice*, std::map<AbilityType, StateValue>> states;
	std::vector<std::function<void(const IDevice&, AbilityType, const StateValue&)>> device_state_update_listeners;

	public:
	static DeviceManager& instance();

	void register_device(IDevice* device);
	const std::vector<IDevice*>& get_devices() const;
	const std::map<AbilityType, StateValue>* get_device_states(const IDevice* device) const;

	void on_device_state_updated(std::function<void(const IDevice&, AbilityType, const StateValue&)> callback);
	void notify_device_state_updated(const IDevice& device, AbilityType type, const StateValue& value);
};
