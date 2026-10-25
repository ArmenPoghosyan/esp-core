#include "includes.h"

#include "device/type/light.h"
#include "led/led.h"
#include "network/device_cert.h"
#include "network/http.h"
#include "network/wifi.h"

#define SERVER_URL "https://devices.aplink.am"

LED led(21);
SimpleLight light("Light 1");

void setup()
{
	Serial.begin(115200);
	Serial.setDebugOutput(true);   // route log_e/log_i (and the TLS layer's messages) to this port

	light.attach_to_led(led)
		.make_passive()
		.make_reportable();

	network.begin();
}

void loop()
{
	static unsigned long last_try = 0;
	// static bool done = false;

	if (network.wifi().is_online() && (last_try == 0 || millis() - last_try > 5000)) {
		last_try = millis();

		const net::HttpResponse response = network.http().get(SERVER_URL "/ping");
		Serial.printf("GET /ping -> %d\n%s\n", response.status, response.body.c_str());

		// done = response.ok();
	}

	delay(3000);
}
