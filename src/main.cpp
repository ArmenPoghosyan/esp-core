#include "includes.h"

#include "device/type/light.h"
#include "led/led.h"

LED led(21);
ColorDimmableLight light("Light 1");

void setup()
{
	light.attach_to_led(led);
	network.begin();
}

void loop()
{
	//
}
