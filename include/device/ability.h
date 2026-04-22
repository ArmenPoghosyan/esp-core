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
	virtual StateValue get_state_value() const = 0;
	virtual bool set_state_value(const StateValue& value) = 0;

	protected:
	IDevice* get_device() const;
	void notify_state_changed(const StateValue& value);
};

template<AbilityType T, typename TState>
class Ability : public AbilityBase {
	protected:
	TState state;

	public:
	Ability(IDevice* device, const TState& initial_state = TState()) : AbilityBase(device, T), state(initial_state) {}

	TState get_state() const {
		return state;
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
};
