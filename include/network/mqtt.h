#pragma once

#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>

namespace net {

/**
 * @brief Receives state changes from the server over MQTT (TLS). Configured in env.h.
 *
 * Iam hands over the channel from every accepted /iam with connect(). The module
 * then subscribes to "<channel>/+": the server publishes {"state": {"brightness": 20, ...}}
 * to "<channel>/<device id>", which is applied to that device with IDevice::from_json().
 *
 * A lost or failed broker connection is retried with the same channel after
 * MQTT_RETRY_MS, the wait doubling on each further failure up to MQTT_RETRY_MAX_MS.
 * When Wi-Fi drops, the channel is dropped with it: the server issues a new one
 * with the next /iam.
 *
 * Runs in the network task, which it blocks while connecting.
 */
class Mqtt {
	public:
	Mqtt();

	void begin();   // call once; Network::begin() does in Wi-Fi mode
	void loop();

	void connect(const String& channel);   // replaces any previous channel; loop() connects

	private:
	void open();    // connect to the broker and subscribe; on failure, schedule the next try
	void close();   // disconnect and forget the channel
	void on_message(char* topic, uint8_t* payload, unsigned int length);

	WiFiClientSecure secure;
	PubSubClient client;

	String channel;                 // empty = none: wait for the next /iam
	unsigned long last_try_ms = 0;
	unsigned long wait_ms = 0;      // before the next try, 0 = at once
};

}
