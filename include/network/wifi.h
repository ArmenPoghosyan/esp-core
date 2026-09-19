#pragma once

#include <Arduino.h>

#include <atomic>
#include <vector>

#include "button/button.h"
#include "led/led.h"
#include "store/credentials_store.h"

namespace net {

	/**
	 * @brief Wi-Fi station with failover across the stored networks, plus the
	 *        captive (setup) mode, a status LED and a mode button. Configured in env.h.
	 *
	 * Networks are tried one at a time in a "pass". A pass starts with a preferred
	 * network and continues in stored order. Nothing ever scans: each join asks
	 * the driver for one network by name, and the driver's answer decides the
	 * next step:
	 *
	 *   connected            -> stay, and remember it as the last working network
	 *   "network not found"  -> next network at once
	 *   any other failure    -> same network again, up to WIFI_CONNECT_ATTEMPTS
	 *   no answer in time    -> next network after WIFI_CONNECT_TIMEOUT_MS
	 *
	 * When every network failed, a new pass starts after WIFI_RETRY_MS.
	 *
	 * The preferred network is the one that worked last (WIFI_PREFER_LAST_GOOD).
	 * When the connected network drops, a new pass starts with that same network:
	 * a brief drop rejoins quickly, and a network that is really gone fails over
	 * as soon as the driver reports it missing.
	 *
	 * LED: slow blink while not connected, fast blink in captive mode, solid when
	 * connected. Holding the button toggles captive mode.
	 */
	class Wifi {
		public:
		Wifi();
		~Wifi();

		void begin();   // call once from setup()
		void loop();    // call from loop()

		void connect();          // start a new pass over the stored networks
		void disconnect();       // radio off; loop() starts a new pass after WIFI_RETRY_MS
		void start_captive();    // open the setup access point and portal
		void stop_captive();     // close it and go back to the stored networks
		void toggle_captive();

		bool is_connected() const;
		bool is_captive() const;
		bool is_online() const;   // connected AND clock synced (TLS-ready)
		String ssid() const;
		String ip() const;

		private:
		enum class State { DISCONNECTED, CONNECTING, CONNECTED, CAPTIVE };

		void start_pass(String preferred);   // by value: callers pass an entry of `networks`, which is rebuilt here
		void start_network();                // begin work on networks[index]
		void join();                         // ask the driver to join networks[index]
		void next_network();
		void on_join_failed(uint8_t reason);
		void on_connected();

		void set_state(State next);          // also drives the LED
		void sync_time();

		State state = State::DISCONNECTED;

		LED led;
		Button button;
		CredentialsStore credentials;

		std::vector<Credentials> networks;   // the current pass, in the order it is tried
		size_t index = 0;                    // network being tried, or the connected one
		uint8_t attempts = 0;                // failed joins of networks[index]
		unsigned long timer_ms = 0;          // start of work on networks[index], or of the retry wait

		// Written from other tasks (Wi-Fi events, button timer), consumed by loop().
		std::atomic<uint8_t> join_failure{0};       // why the latest join failed, 0 = it did not
		std::atomic<bool> toggle_requested{false};
		size_t event_id = 0;                        // WiFi.onEvent() registration, 0 = none

		unsigned long blink_ms = 0;   // current LED blink rate, 0 = not blinking
		bool time_started = false;    // NTP is started once
	};

}
