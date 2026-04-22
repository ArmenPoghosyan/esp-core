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

using StateValue = std::variant<std::monostate, bool, uint8_t, uint16_t, uint32_t, int32_t, float>;


