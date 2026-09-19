#include "network/wifi.h"

#include <WiFi.h>

#include <time.h>

#include <algorithm>

#include "env.h"
#include "network/captive_portal.h"
#include "network/network.h"

namespace net {

	namespace {
		// "<prefix>-<MAC suffix>", e.g. "APLink-926C".
		String device_id(const char* prefix) {
			char buf[32];
			snprintf(buf, sizeof(buf), "%s-%04X", prefix, static_cast<uint16_t>(ESP.getEfuseMac() & 0xFFFF));
			return String(buf);
		}
	}

	Wifi::Wifi() : led(WIFI_LED_PIN), button(WIFI_BUTTON_PIN) {}

	Wifi::~Wifi() {
		if (event_id) {
			WiFi.removeEvent(event_id);   // the callback captures `this`
		}
	}

	// ---------------------------------------------------------------- lifecycle

	void Wifi::begin() {
		if (event_id) {
			return;   // already started
		}

		WiFi.persistent(false);
		WiFi.setSleep(false);           // keep the radio responsive: faster joins, fewer drops
		WiFi.setAutoReconnect(false);   // every join is issued by loop(), so it always knows what the radio is doing

		// Runs in the Wi-Fi event task: only note why the join failed, loop() decides
		// what to do about it. ASSOC_LEAVE means we left on purpose (switching to
		// another network), so it is not a failure.
		const auto note_failure = [this](arduino_event_id_t, arduino_event_info_t info) {
			const uint8_t reason = info.wifi_sta_disconnected.reason;
			if (reason != WIFI_REASON_ASSOC_LEAVE) {
				join_failure = reason ? reason : static_cast<uint8_t>(WIFI_REASON_UNSPECIFIED);
			}
		};
		event_id = WiFi.onEvent(note_failure, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

		button.onLongPress(WIFI_BUTTON_HOLD_MS, [this]() { toggle_requested = true; });

		connect();   // with nothing stored this simply waits
	}

	void Wifi::loop() {
		if (toggle_requested.exchange(false)) {
			toggle_captive();
		}

		switch (state) {
			case State::CONNECTING:
				// The SSID check rejects a WL_CONNECTED left over from the previous
				// network while the driver is still switching.
				if (WiFi.status() == WL_CONNECTED && WiFi.SSID() == networks[index].ssid) {
					on_connected();
				} else if (const uint8_t reason = join_failure.exchange(0)) {
					on_join_failed(reason);
				} else if (millis() - timer_ms >= WIFI_CONNECT_TIMEOUT_MS) {
					next_network();   // the AP answers but the join never completes
				}
				break;

			case State::CONNECTED:
				if (WiFi.status() != WL_CONNECTED) {
					log_i("lost '%s'", networks[index].ssid.c_str());
					start_pass(networks[index].ssid);   // rejoin it first; fail over if it is gone
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

	// ---------------------------------------------------------------- one pass over the stored networks

	void Wifi::connect() {
		start_pass(WIFI_PREFER_LAST_GOOD ? credentials.get_last_connected() : String());
	}

	void Wifi::start_pass(String preferred) {
		networks = credentials.get_credentials();
		index = 0;

		if (networks.empty()) {
			set_state(State::DISCONNECTED);   // nothing stored: wait for the portal to add a network
			timer_ms = millis();
			return;
		}

		// The preferred network goes first; the others keep their stored order.
		const auto it = std::find_if(networks.begin(), networks.end(), [&preferred](const Credentials& c) { return c.ssid == preferred; });
		if (it != networks.end()) {
			std::rotate(networks.begin(), it, it + 1);
		}

		WiFi.setHostname(device_id(WIFI_AP_PREFIX).c_str());
		WiFi.mode(WIFI_STA);
		start_network();
	}

	void Wifi::start_network() {
		set_state(State::CONNECTING);
		attempts = 0;
		timer_ms = millis();

		log_i("trying '%s' (%u of %u)", networks[index].ssid.c_str(), static_cast<unsigned>(index + 1), static_cast<unsigned>(networks.size()));
		join();
	}

	void Wifi::join() {
		join_failure = 0;   // a failure noted before this point belongs to an earlier join
		WiFi.begin(networks[index].ssid.c_str(), networks[index].password.c_str());
	}

	void Wifi::next_network() {
		index++;
		if (index < networks.size()) {
			start_network();
			return;
		}

		// The whole pass failed: wait, then start over.
		WiFi.disconnect();   // stop a join that may still be in flight, so this state really is idle
		set_state(State::DISCONNECTED);
		timer_ms = millis();
	}

	void Wifi::on_join_failed(uint8_t reason) {
		log_i("'%s' failed after %lu ms: %s", networks[index].ssid.c_str(), millis() - timer_ms,
			WiFi.disconnectReasonName(static_cast<wifi_err_reason_t>(reason)));

		// "Not found" will not change within seconds, so move on at once. Any other
		// reason means the AP answered but the join failed, which is often
		// transient, so ask again. The timer keeps running: WIFI_CONNECT_TIMEOUT_MS
		// still bounds the total time spent on this network.
		const bool missing = reason == WIFI_REASON_NO_AP_FOUND;
		if (!missing && ++attempts < WIFI_CONNECT_ATTEMPTS) {
			join();
		} else {
			next_network();
		}
	}

	void Wifi::on_connected() {
		log_i("connected to '%s' after %lu ms", networks[index].ssid.c_str(), millis() - timer_ms);
		set_state(State::CONNECTED);

		if (WIFI_PREFER_LAST_GOOD) {
			credentials.set_last_connected(networks[index].ssid);   // writes flash only when it changed
		}

		sync_time();   // start NTP so TLS cert dates validate
	}

	void Wifi::disconnect() {
		WiFi.disconnect(true);   // true: also turn the radio off
		set_state(State::DISCONNECTED);
		timer_ms = millis();
	}

	// ---------------------------------------------------------------- captive (setup) mode

	void Wifi::start_captive() {
		if (state == State::CONNECTING) {
			WiFi.disconnect();   // a join hopping across channels would make the setup AP unstable
		}

		// AP_STA keeps a current connection alive while the setup portal is open.
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
		connect();   // the portal may have changed the stored networks
	}

	void Wifi::toggle_captive() {
		if (state == State::CAPTIVE) {
			stop_captive();
		} else {
			start_captive();
		}
	}

	// ---------------------------------------------------------------- status

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

	String Wifi::ssid() const {
		return WiFi.SSID();
	}

	String Wifi::ip() const {
		return WiFi.localIP().toString();
	}

	void Wifi::sync_time() {
		if (time_started) {
			return;   // SNTP keeps itself updated once started
		}

		configTime(0, 0, WIFI_NTP_SERVER);   // UTC; SNTP updates the clock in the background
		time_started = true;
	}

	void Wifi::set_state(State next) {
		state = next;

		if (state == State::CONNECTED) {
			led.stop_blink();
			led.on();
			blink_ms = 0;
			return;
		}

		// Restart the blink only when its rate changes, so stepping through the
		// networks keeps a steady rhythm.
		const unsigned long wanted = (state == State::CAPTIVE) ? WIFI_BLINK_CAPTIVE_MS : WIFI_BLINK_IDLE_MS;
		if (wanted != blink_ms) {
			led.blink(wanted);
			blink_ms = wanted;
		}
	}

}
