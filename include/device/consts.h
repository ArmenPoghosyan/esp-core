#pragma once

#include <cstdint>
#include <variant>

enum class DeviceType : uint8_t {
	LIGHT = 1,
	SWITCH,
	SENSOR,
};

enum class AbilityType : uint8_t {
	ON_OFF = 1,
	BRIGHTNESS,
	COLOR,
};

enum JsonDeviceFlags : uint8_t {
	JSON_DEVICE_INFO = 1 << 0,
	JSON_DEVICE_ABILITIES = 1 << 1,
	JSON_DEVICE_STATE = 1 << 2,

	JSON_DEVICE_ALL = JSON_DEVICE_INFO | JSON_DEVICE_ABILITIES | JSON_DEVICE_STATE
};

using StateValue = std::variant<std::monostate, bool, uint8_t, uint16_t, uint32_t, int32_t, float>;

template <typename T>
T convert_state(const StateValue& state) {
	return std::get<T>(state);
}
