#pragma once

#include "device/consts.h"

#include <functional>
#include <vector>

class AbilityBase;

struct AbilityState {
	AbilityType type;
	StateValue value;
};

class IDevice {
	private:
	friend class AbilityBase;

	DeviceType type;
	const char* name;
	std::vector<AbilityBase*> abilities;
	std::vector<std::function<void(AbilityType, const StateValue&)>> state_update_listeners;

	public:
	IDevice(DeviceType type, const char* name);
	virtual ~IDevice();

	/**
	 * @brief Get the type of the device.
	 */
	DeviceType get_type() const;

	/**
	 * @brief Get the name of the device.
	 */
	const char* get_name() const;

	/**
	 * @brief Registers a new ability with the device.
	 */
	void register_ability(AbilityBase* ability);

	/**
	 * @brief Get the list of abilities of the device.
	 */
	const std::vector<AbilityBase*>& get_abilities() const;

	/**
	 * @brief Get the current states of all abilities of the device.
	 */
	std::vector<AbilityState> get_ability_states() const;

	/**
	 * @brief Registers a callback to be invoked whenever the state of an ability is updated.
	 */
	void on_state_updated(std::function<void(AbilityType, const StateValue&)> callback);

	protected:
	/**
	 * @brief Notifies all registered listeners that the state of an ability has been updated.
	 */
	void notify_state_updated(AbilityType type, const StateValue& value);
};

template<typename... Abilities>
class GenericDevice : public IDevice, public Abilities... {
	public:
	GenericDevice(DeviceType type, const char* name) : IDevice(type, name), Abilities(this)... {
		//
	}

	/**
	 * @brief Registers the device with the device manager.
	 */
	void register_device() {
		device_manager.register_device(this);
	}
};
