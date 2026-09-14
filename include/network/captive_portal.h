#pragma once

#include <Arduino.h>

#include <WebServer.h>

#include <functional>

#include "store/credentials_store.h"

namespace net {

/**
 * @brief Serves the setup page while in captive mode. Lets the user add and
 *        remove multiple Wi-Fi credentials, then press "Connect" to finish.
 *
 * Assumes the SoftAP is already up (the Wifi module brings it up). Uses the
 * lazily-created Dns module for the captive redirect.
 */
class CaptivePortal {
	public:
	void begin();
	void loop();
	void stop();
	bool is_running() const;

	/** Called when the user presses "Connect" on the page. */
	void on_finished(std::function<void()> callback);

	private:
	void handle_root();
	void handle_add();
	void handle_remove();
	void handle_finish();
	void redirect_home();
	void redirect_to_portal();

	WebServer server{80};
	CredentialsStore credentials;
	bool running = false;
	bool routes_registered = false;
	std::function<void()> finished_cb;
};

}
