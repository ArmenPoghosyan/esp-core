#include "network/mqtt.h"

#include <ArduinoJson.h>

#include <algorithm>

#include "device/device.h"
#include "device/manager.h"
#include "env.h"
#include "network/network.h"
#include "network/wifi.h"

// The Mozilla root-CA bundle, as in http.cpp: the broker's certificate is verified like any HTTPS server's.
extern const uint8_t ca_cert_bundle[] asm("x509_crt_bundle");

namespace net {

	Mqtt::Mqtt() : client(secure) {}

	void Mqtt::begin() {
		secure.setCACertBundle(ca_cert_bundle);
		secure.setTimeout(MQTT_TIMEOUT_S);
		secure.setHandshakeTimeout(MQTT_TIMEOUT_S);

		client.setServer(MQTT_HOST, MQTT_PORT);
		client.setBufferSize(MQTT_BUFFER_SIZE);
		client.setCallback([this](char* topic, uint8_t* payload, unsigned int length) {
			on_message(topic, payload, length);
		});

		// Fires in the network task, like loop(), so no locking is needed.
		network.wifi().on_disconnected([this]() { close(); });
	}

	void Mqtt::loop() {
		if (channel.isEmpty()) {
			return;
		}

		if (client.connected()) {
			client.loop();   // delivers messages to on_message() and keeps the connection alive
			return;
		}

		if (millis() - last_try_ms >= wait_ms) {
			open();
		}
	}

	void Mqtt::connect(const String& new_channel) {
		close();   // the server no longer publishes to the previous channel
		channel = new_channel;
		wait_ms = 0;
	}

	void Mqtt::open() {
		const String topic = channel + "/+";

		if (client.connect(device_manager.get_mac_address().c_str(), MQTT_USERNAME, MQTT_PASSWORD)
			&& client.subscribe(topic.c_str())) {
			log_i("MQTT subscribed to %s", topic.c_str());
			wait_ms = 0;   // a later drop is retried at once
			return;
		}

		const int state = client.state();
		client.disconnect();   // connected but not subscribed is no use: start over next time

		last_try_ms = millis();
		wait_ms = wait_ms ? std::min<unsigned long>(wait_ms * 2, MQTT_RETRY_MAX_MS) : MQTT_RETRY_MS;
		log_e("MQTT connect failed (state %d), retry in %lu ms", state, wait_ms);
	}

	void Mqtt::close() {
		if (client.connected()) {
			client.disconnect();
		}
		channel = "";
	}

	void Mqtt::on_message(char* topic, uint8_t* payload, unsigned int length) {
		// The topic is "<channel>/<device id>".
		const char* id = strrchr(topic, '/');
		IDevice* device = id ? device_manager.get_device(strtoull(id + 1, nullptr, 10)) : nullptr;
		if (!device) {
			log_w("MQTT %s: no such device", topic);
			return;
		}

		JsonDocument doc;
		if (deserializeJson(doc, payload, length) != DeserializationError::Ok) {
			log_w("MQTT %s: not JSON", topic);
			return;
		}

		device->from_json(doc["state"].as<JsonObjectConst>());
	}

}
