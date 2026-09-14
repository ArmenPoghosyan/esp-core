#include "network/wifi.h"

#include <WiFi.h>

#include <time.h>

#include "env.h"
#include "network/captive_portal.h"
#include "network/network.h"

namespace net {

namespace {
	// "<prefix>-<MAC suffix>", e.g. "APSuite-926C" or "Light-926C".
	String device_id(const char* prefix) {
		char buf[32];
		snprintf(buf, sizeof(buf), "%s-%04X", prefix, static_cast<uint16_t>(ESP.getEfuseMac() & 0xFFFF));
		return String(buf);
	}
}

Wifi::Wifi() : led(WIFI_LED_PIN), button(WIFI_BUTTON_PIN) {}

void Wifi::begin() {
	WiFi.persistent(false);
	WiFi.setAutoReconnect(false);   // we manage retries ourselves

	// Set the hostname when the STA starts — the reliable point on esp32 2.0.x
	// (setting it between mode() and begin() often no-ops).
	WiFi.onEvent([](WiFiEvent_t event) {
		if (event == ARDUINO_EVENT_WIFI_STA_START) {
			WiFi.setHostname(device_id(WIFI_AP_PREFIX).c_str());
		}
	});

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
				sync_time();   // start NTP so TLS cert dates validate
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

	WiFi.mode(WIFI_STA);  // set to station mode before connecting
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
	// AP_STA so the captive portal can still scan for networks.
	WiFi.mode(WIFI_AP_STA);
	WiFi.softAP(device_id(WIFI_AP_PREFIX).c_str());   // "<manufacturer>-<MAC suffix>"
	set_state(State::CAPTIVE);

	network.captive_portal().on_finished([this]() { toggle_requested = true; });
	network.captive_portal().begin();
}

void Wifi::stop_captive() {
	network.captive_portal().stop();
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

bool Wifi::is_online() const {
	// Connected and the clock has been set (past 2021) -> HTTPS can verify certs.
	return is_connected() && time(nullptr) > 1609459200;
}

void Wifi::sync_time() {
	configTime(0, 0, WIFI_NTP_SERVER);   // UTC; SNTP updates the clock in the background
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
