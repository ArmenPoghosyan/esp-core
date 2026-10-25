#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <memory>

namespace net {
	class Wifi;
	class Dns;
	class CaptivePortal;
	class Http;
	class EspNow;
	class Iam;
	class Mqtt;
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
	net::EspNow& now();
	net::Iam& iam();
	net::Mqtt& mqtt();

	/** Start the configured modules and the task that drives them. Call once from setup(). */
	void begin();

	/** Drive every module that has been created. Called by the network task. */
	void loop();

	private:
	static void run(void* self);   // the network task: loop() forever
	TaskHandle_t task = nullptr;

	std::unique_ptr<net::Wifi> wifi_;
	std::unique_ptr<net::Dns> dns_;
	std::unique_ptr<net::CaptivePortal> captive_portal_;
	std::unique_ptr<net::Http> http_;
	std::unique_ptr<net::EspNow> now_;
	std::unique_ptr<net::Iam> iam_;
	std::unique_ptr<net::Mqtt> mqtt_;
};

// The single, globally-available network instance.
inline Network network;
