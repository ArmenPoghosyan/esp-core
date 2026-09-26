#include "network/network.h"

#include "env.h"
#include "network/captive_portal.h"
#include "network/dns.h"
#include "network/esp_now.h"
#include "network/http.h"
#include "network/wifi.h"

Network::Network() = default;
Network::~Network() = default;

net::Wifi& Network::wifi() {
	if (!wifi_) {
		wifi_ = std::unique_ptr<net::Wifi>(new net::Wifi());
	}
	return *wifi_;
}

net::Dns& Network::dns() {
	if (!dns_) {
		dns_ = std::unique_ptr<net::Dns>(new net::Dns());
	}
	return *dns_;
}

net::CaptivePortal& Network::captive_portal() {
	if (!captive_portal_) {
		captive_portal_ = std::unique_ptr<net::CaptivePortal>(new net::CaptivePortal());
	}
	return *captive_portal_;
}

net::Http& Network::http() {
	if (!http_) {
		http_ = std::unique_ptr<net::Http>(new net::Http());
	}
	return *http_;
}

net::EspNow& Network::now() {
	if (!now_) {
		now_ = std::unique_ptr<net::EspNow>(new net::EspNow());
	}
	return *now_;
}

void Network::begin() {
	if (task) {
		return;   // already started
	}

	#if DEVICE_MODE == DEVICE_MODE_WIFI
	wifi().begin();
	#endif

	// A task rather than a timer: the modules need a real stack (the setup
	// portal's web server alone outgrows the 2 KB timer task) and may block briefly.
	xTaskCreate(&Network::run, "net", NETWORK_TASK_STACK, this, NETWORK_TASK_PRIORITY, &task);
}

void Network::run(void* self) {
	Network& network = *static_cast<Network*>(self);

	for (;;) {
		network.loop();
		vTaskDelay(pdMS_TO_TICKS(NETWORK_LOOP_MS));
	}
}

void Network::loop() {
	if (wifi_) {
		wifi_->loop();
	}
	if (dns_) {
		dns_->loop();
	}
	if (captive_portal_) {
		captive_portal_->loop();
	}
	if (now_) {
		now_->loop();
	}
}
