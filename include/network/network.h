#pragma once

#include <memory>

namespace net {
	class Wifi;
	class Dns;
	class CaptivePortal;
	class Http;
}

/**
 * @brief Project-wide network hub with lazily-created modules.
 *
 * A module (wifi, dns, captive_portal, ...) is constructed the first time it is
 * requested, so unused modules cost nothing. Use the global `network` object,
 * e.g. `network.wifi().connect();`.
 */
class Network {
	public:
	Network();
	~Network();

	net::Wifi& wifi();
	net::Dns& dns();
	net::CaptivePortal& captive_portal();
	net::Http& http();

	/** Drive every module that has been created. Call from loop(). */
	void loop();

	private:
	std::unique_ptr<net::Wifi> wifi_;
	std::unique_ptr<net::Dns> dns_;
	std::unique_ptr<net::CaptivePortal> captive_portal_;
	std::unique_ptr<net::Http> http_;
};

// The single, globally-available network instance.
inline Network network;
