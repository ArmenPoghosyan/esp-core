#include "network/wifi.h"

#include <WiFi.h>

#include "env.h"

namespace net {

Wifi::Wifi() : led(WIFI_LED_PIN), button(WIFI_BUTTON_PIN) {}

void Wifi::begin() {
	WiFi.persistent(false);
	WiFi.setAutoReconnect(false);   // we manage retries ourselves

	button.onLongPress(WIFI_BUTTON_HOLD_MS, [this]() { toggle_requested = true; });

	connect();   // if nothing is stored this simply stays disconnected
}

void Wifi::loop() {
	if (toggle_requested) {
		toggle_requested = false;
		toggle_captive();
	}

	switch (state) {
		case State::CONNECTING:
			if (WiFi.status() == WL_CONNECTED) {
				set_state(State::CONNECTED);
			} else if (millis() - timer_ms >= WIFI_CONNECT_TIMEOUT_MS) {
				index++;
				if (index < networks.size()) {
					try_current_credential();   // next backup network
				} else {
					set_state(State::DISCONNECTED);
					timer_ms = millis();        // start the retry cooldown
				}
			}
			break;

		case State::CONNECTED:
			if (WiFi.status() != WL_CONNECTED) {
				connect();                      // dropped -> reconnect from the top
			}
			break;

		case State::DISCONNECTED:
			if (millis() - timer_ms >= WIFI_RETRY_MS) {
				connect();
			}
			break;

		case State::CAPTIVE:
			break;
	}
}

void Wifi::connect() {
	networks = credentials.get_credentials();
	index = 0;

	if (networks.empty()) {
		set_state(State::DISCONNECTED);   // no networks -> idle (slow blink), no captive
		timer_ms = millis();
		return;
	}

	WiFi.mode(WIFI_STA);
	try_current_credential();
}

void Wifi::try_current_credential() {
	set_state(State::CONNECTING);
	WiFi.begin(networks[index].ssid.c_str(), networks[index].password.c_str());
	timer_ms = millis();
}

void Wifi::disconnect() {
	WiFi.disconnect(true);
	set_state(State::DISCONNECTED);
	timer_ms = millis();
}

void Wifi::start_captive() {
	// Name the AP "<manufacturer>-<MAC suffix>" so every device is unique.
	char name[32];
	const uint16_t mac_suffix = static_cast<uint16_t>(ESP.getEfuseMac() & 0xFFFF);
	snprintf(name, sizeof(name), "%s-%04X", WIFI_AP_PREFIX, mac_suffix);

	// AP_STA so a captive-portal module can still scan for networks later.
	WiFi.mode(WIFI_AP_STA);
	WiFi.softAP(name);
	set_state(State::CAPTIVE);
}

void Wifi::stop_captive() {
	WiFi.softAPdisconnect(true);
	WiFi.mode(WIFI_STA);
}

void Wifi::toggle_captive() {
	if (state == State::CAPTIVE) {
		stop_captive();
		connect();
	} else {
		start_captive();
	}
}

bool Wifi::is_connected() const {
	return state == State::CONNECTED;
}

bool Wifi::is_captive() const {
	return state == State::CAPTIVE;
}

String Wifi::ssid() const {
	return WiFi.SSID();
}

String Wifi::ip() const {
	return WiFi.localIP().toString();
}

void Wifi::set_state(State next) {
	state = next;
	apply_led();
}

void Wifi::apply_led() {
	switch (state) {
		case State::CONNECTED:
			led.stop_blink();
			led.on();
			break;
		case State::CAPTIVE:
			led.blink(WIFI_BLINK_CAPTIVE_MS);
			break;
		default:   // DISCONNECTED / CONNECTING
			led.blink(WIFI_BLINK_IDLE_MS);
			break;
	}
}

}
