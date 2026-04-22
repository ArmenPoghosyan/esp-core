#include <Arduino.h>

#include "device/abilities/brightness.h"
#include "device/abilities/color.h"
#include "device/types/light.h"
#include "device/types/switch.h"

#include "device/manager.h"

Light<OnOff, Brightness, Color> light1("Living Room Light");
BasicSwitch wall_switch("Wall Switch");

static void print_state_value(const StateValue& value) {
	if (const bool* v = std::get_if<bool>(&value)) {
		Serial.print(*v ? "true" : "false");
		return;
	}

	if (const uint8_t* v = std::get_if<uint8_t>(&value)) {
		Serial.print(*v);
		return;
	}

	if (const uint16_t* v = std::get_if<uint16_t>(&value)) {
		Serial.print(*v);
		return;
	}

	if (const uint32_t* v = std::get_if<uint32_t>(&value)) {
		Serial.print("0x");
		Serial.print(*v, HEX);
		return;
	}

	if (const int32_t* v = std::get_if<int32_t>(&value)) {
		Serial.print(*v);
		return;
	}

	if (const float* v = std::get_if<float>(&value)) {
		Serial.print(*v);
		return;
	}

	Serial.print("null");
}

void setup()
{
	Serial.begin(9600);
	pinMode(LED_BUILTIN, OUTPUT);

	light1.on_state_updated([](AbilityType ability, const StateValue& value)
		{
			Serial.print("State updated: ");
	// 		Serial.print(device.get_name());
			Serial.print(" ability=");
			Serial.print(static_cast<int>(ability));
			Serial.print(" value=");
			print_state_value(value);
			Serial.println();

			switch (ability)
			{
				case AbilityType::ON_OFF:
					Serial.print("Light ON/OFF state changed: ");
					digitalWrite(LED_BUILTIN, std::get<bool>(value) ? HIGH : LOW);
					break;
			}
		}
	);

	// DeviceManager::instance().on_device_state_updated([](const IDevice& device, AbilityType ability, const StateValue& value)
	// 	{
	// 		Serial.print("State updated: ");
	// 		Serial.print(device.get_name());
	// 		Serial.print(" ability=");
	// 		Serial.print(static_cast<int>(ability));
	// 		Serial.print(" value=");
	// 		print_state_value(value);
	// 		Serial.println();
	// 	}
	// );

	// delay(5000); // Wait for the serial monitor to initialize

	light1.turn_on();
	light1.set_brightness(75);
	light1.set_color(0x0000FF00);
	wall_switch.turn_off();
}

void loop()
{
	light1.toggle_on_off();
	delay(2000);
}
