#pragma once

#include "device/device.h"

class AbilityBase {
	private:
	IDevice* device;
	AbilityType type;

	public:
	AbilityBase(IDevice* device, AbilityType type);
	virtual ~AbilityBase() = default;

	AbilityType get_type() const;
	virtual const char* get_name() const = 0;
	virtual StateValue get_state_value() const = 0;
	virtual bool set_state_value(const StateValue& value) = 0;

	/** Sets the state from a JSON value; false when the value is not of this ability's type. */
	virtual bool set_state_from_json(JsonVariantConst value) = 0;

	protected:
	IDevice* get_device() const;
	void notify_state_changed(const StateValue& value);
};

template<AbilityType T, typename TState>
class Ability : public AbilityBase {
	protected:
	TState state;
	const char* name;

	public:
	Ability(IDevice* device, const TState& initial_state = TState(), const char* ability_name = "") : AbilityBase(device, T), state(initial_state), name(ability_name) {}

	TState get_state() const {
		return state;
	}

	const char* get_name() const override {
		return name;
	}

	void set_state(const TState& value) {
		if (state == value) {
			return;
		}

		state = value;
		notify_state_changed(StateValue(state));
	}

	StateValue get_state_value() const override {
		return StateValue(state);
	}

	bool set_state_value(const StateValue& value) override {
		const TState* typed_value = std::get_if<TState>(&value);
		if (!typed_value) {
			return false;
		}

		set_state(*typed_value);
		return true;
	}

	bool set_state_from_json(JsonVariantConst value) override {
		if (!value.is<TState>()) {
			return false;   // wrong JSON type, or an integer outside TState's range
		}

		return set_state_value(StateValue(value.as<TState>()));
	}
};
