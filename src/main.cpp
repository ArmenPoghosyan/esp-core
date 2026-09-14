#include <Arduino.h>

#include "network/network.h"
#include "network/wifi.h"

void setup()
{
	Serial.begin(115200);
	network.wifi().begin();
}

void loop()
{
	network.loop();
}
