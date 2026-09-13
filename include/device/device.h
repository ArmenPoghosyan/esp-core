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
	 * @brief Get the type of the device
	 */
	DeviceType get_type() const;

	/**
	 * @brief Get the name of the device
	 */
	const char* get_name() const;

	void register_ability(AbilityBase* ability);
	const std::vector<AbilityBase*>& get_abilities() const;
	std::vector<AbilityState> get_ability_states() const;

	void on_state_updated(std::function<void(AbilityType, const StateValue&)> callback);

	protected:
	void notify_state_updated(AbilityType type, const StateValue& value);
};

template<typename... Abilities>
class GenericDevice : public IDevice, public Abilities... {
	public:
	GenericDevice(DeviceType type, const char* name) : IDevice(type, name), Abilities(this)... {
		//
	}
};
