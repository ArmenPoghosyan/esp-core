#include "network/dns.h"

namespace net {

void Dns::start(const IPAddress& ip, uint16_t port) {
	if (running) {
		return;
	}

	server.start(port, "*", ip);   // "*" -> resolve every hostname to ip
	running = true;
}

void Dns::stop() {
	if (!running) {
		return;
	}

	server.stop();
	running = false;
}

void Dns::loop() {
	if (running) {
		server.processNextRequest();
	}
}

bool Dns::is_running() const {
	return running;
}

}
