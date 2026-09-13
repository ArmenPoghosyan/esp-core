#include <Arduino.h>

#include "device/type/light.h"
#include "device/ability/on_off.h"
#include "device/ability/brightness.h"
#include "led/led.h"

Light<OnOff, Brightness> light("Light 1");

LED led;

void setup()
{
	Serial.begin(115200);

	light.on_state_updated([](AbilityType ability, StateValue state) {
		switch(ability) {
			case AbilityType::ON_OFF:
				{
					bool is_on = std::get<bool>(state);
					Serial.print("ON_OFF state updated: ");
					Serial.println(is_on ? "on" : "off");
					led.set(is_on);
				}
				break;
			default: break;
		}
	});
}

void loop()
{
	delay(500);

	light.toggle_on_off();
}
