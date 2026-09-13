#include <Arduino.h>

#include "network/network.h"
#include "network/wifi.h"

#include "store/credentials_store.h"

	CredentialsStore store;

void setup()
{
	Serial.begin(115200);

	// store.clear_credentials();

	network.wifi().begin();
}

void loop()
{
	network.loop();
}
