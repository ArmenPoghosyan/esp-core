#include "includes.h"

#include "device/type/light.h"
#include "led/led.h"

LED led(21);
DimmableLight light("Light 1");

void setup()
{
	init_system();
	light.attach_to_led(led);
	light.turn_on();
}

void loop()
{
	loop_system();
}
