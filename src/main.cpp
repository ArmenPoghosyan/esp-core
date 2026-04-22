#include <Arduino.h>

#include "device/abilities/brightness.h"
#include "device/abilities/color.h"
#include "device/types/light.h"
#include "device/types/switch.h"

#include "device/manager.h"

Light<OnOff, Brightness, Color> light1("Living Room Light");
BasicSwitch wall_switch("Wall Switch");

void setup()
{
	light1.on_state_updated([](AbilityType ability, const StateValue& value)
		{
			switch (ability)
			{
				case AbilityType::ON_OFF:
					Serial.print("Light ON/OFF state changed: ");
					digitalWrite(LED_BUILTIN, std::get<bool>(value) ? HIGH : LOW);
					break;
			}
		}
	);
}

void loop()
{
	light1.toggle_on_off();
	delay(1000);
}
