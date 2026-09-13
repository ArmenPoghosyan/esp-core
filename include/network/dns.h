#pragma once

#include <Arduino.h>

#include <DNSServer.h>
#include <IPAddress.h>

namespace net {

/**
 * @brief Tiny wrapper around DNSServer. In captive mode it answers every query
 *        with the device IP, which makes phones pop the setup page.
 */
class Dns {
	public:
	void start(const IPAddress& ip, uint16_t port = 53);
	void stop();
	void loop();
	bool is_running() const;

	private:
	DNSServer server;
	bool running = false;
};

}
