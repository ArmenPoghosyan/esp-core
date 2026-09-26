#include "includes.h"

#include "device/type/light.h"
#include "led/led.h"

LED led(21);
DimmableLight light("Light 1");

void setup()
{
	network.begin();
	light.attach_to_led(led);
}

void loop()
{
	//
}
