#include "includes.h"

#include "device/type/light.h"
#include "led/led.h"

LED led(21);
DimmableLight light("Light 1");

void setup()
{
	init_system();
	light.turn_on();
	light.set_brightness(50);
	light.attach_to_led(led);
}

void loop()
{
	loop_system();

	Serial.println(device_manager.to_json(JSON_DEVICE_ALL));

	Serial.println();
	delay(1000);
}
