#pragma once

#include <memory>

namespace net {
	class Wifi;
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

	/** Drive every module that has been created. Call from loop(). */
	void loop();

	private:
	std::unique_ptr<net::Wifi> wifi_;
};

// The single, globally-available network instance.
inline Network network;
