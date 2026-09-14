#pragma once

#include <Arduino.h>

#include <vector>

#include "button/button.h"
#include "led/led.h"
#include "store/credentials_store.h"

namespace net {

/**
 * @brief Handles all Wi-Fi behaviour: connecting to stored networks, captive
 *        (AP) mode, and a status LED + button, all configured from env.h.
 *
 * LED: blinks slowly when not connected, fast in captive mode, solid when
 * connected. Holding the button toggles captive mode.
 */
class Wifi {
	public:
	Wifi();

	void begin();
	void loop();

	void connect();          // try stored credentials, primary -> backups
	void disconnect();
	void start_captive();    // bring up the SoftAP
	void stop_captive();
	void toggle_captive();

	bool is_connected() const;
	bool is_captive() const;
	bool is_online() const;   // connected AND clock synced (TLS-ready)
	String ssid() const;
	String ip() const;

	private:
	enum class State { DISCONNECTED, CONNECTING, CONNECTED, CAPTIVE };

	void set_state(State next);
	void apply_led();
	void try_current_credential();
	void sync_time();

	State state = State::DISCONNECTED;

	LED led;
	Button button;
	CredentialsStore credentials;

	std::vector<Credentials> networks;
	size_t index = 0;
	unsigned long timer_ms = 0;

	unsigned long led_interval = 0;   // current blink interval (0 = solid/off)
	bool time_started = false;        // NTP started once

	volatile bool toggle_requested = false;
};

}
