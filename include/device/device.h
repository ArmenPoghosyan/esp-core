#pragma once

#include "device/consts.h"
#include "device/manager.h"

#include <Arduino.h>
#include <ArduinoJson.h>

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
	uint64_t id;
	std::vector<AbilityBase*> abilities;
	std::vector<std::function<void(AbilityType, const StateValue&)>> state_update_listeners;

	uint64_t generate_id() const;

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
	 * @brief Get the unique id of the device.
	 */
	uint64_t get_id() const;

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

	/**
	 * @brief Writes the device into `json`, e.g. an object inside a larger document.
	 * @param flags Which parts to write: JSON_DEVICE_INFO, JSON_DEVICE_ABILITIES
	 *              and JSON_DEVICE_STATE, combined with |.
	 */
	void to_json(JsonObject json, uint8_t flags = JSON_DEVICE_ALL) const;

	/**
	 * @brief The same, as serialized JSON text.
	 */
	String to_json(uint8_t flags = JSON_DEVICE_ALL) const;

	/**
	 * @brief Applies a state object as written by to_json(JSON_DEVICE_STATE),
	 *        e.g. {"on_off": true, "brightness": 50}.
	 *
	 * Only abilities the device has are changed: an unknown name, or a value
	 * of the wrong type for the ability, is skipped.
	 * @return true when every item was applied.
	 */
	bool from_json(JsonObjectConst state);

	/**
	 * @brief The same, from JSON text. false when the text is not a JSON object.
	 */
	bool from_json(const String& state);

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
