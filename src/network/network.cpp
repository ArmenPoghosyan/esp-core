#include "network/network.h"

#include "network/captive_portal.h"
#include "network/dns.h"
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
}
