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
	String ssid() const;
	String ip() const;

	private:
	enum class State { DISCONNECTED, CONNECTING, CONNECTED, CAPTIVE };

	void set_state(State next);
	void apply_led();
	void try_current_credential();

	State state = State::DISCONNECTED;

	LED led;
	Button button;
	CredentialsStore credentials;

	std::vector<Credentials> networks;
	size_t index = 0;
	unsigned long timer_ms = 0;

	volatile bool toggle_requested = false;
};

}
