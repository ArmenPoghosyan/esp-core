#include <Arduino.h>
#include "consts.h"
#include "env.h"

#include "network/network.h"

#if DEVICE_MODE == DEVICE_MODE_WIFI
#include "network/wifi.h"
#endif

void init_system() {
	Serial.begin(115200);

	#if DEVICE_MODE == DEVICE_MODE_WIFI
	network.wifi().begin();
	#endif
}

void loop_system() {
	#if DEVICE_MODE == DEVICE_MODE_WIFI
	network.loop();
	#endif
}
