#include "network/network.h"

#include "network/wifi.h"

Network::Network() = default;
Network::~Network() = default;

net::Wifi& Network::wifi() {
	if (!wifi_) {
		wifi_ = std::unique_ptr<net::Wifi>(new net::Wifi());
	}
	return *wifi_;
}

void Network::loop() {
	if (wifi_) {
		wifi_->loop();
	}
}
