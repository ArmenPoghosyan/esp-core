#include "includes.h"

#include "device/type/light.h"
#include "led/led.h"

LED led(21);
ColorDimmableLight light("Light 1");

void setup()
{
	Serial.begin(115200);
	Serial.setDebugOutput(true);   // route log_e/log_i (and the TLS layer's messages) to this port

	light.attach_to_led(led);

	network.begin();
}

void loop()
{
	//
}
