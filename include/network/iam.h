#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

namespace net {

/**
 * @brief Introduces the device to the server: POST SERVER_URL/iam with
 *        device_manager.to_json(), on every Wi-Fi connect and reconnect.
 *        Configured in env.h.
 *
 * The server answers {"status": true, "state": {"<device id>": {...}, ...}}.
 * Each state is applied to the device with that id, as IDevice::from_json().
 *
 * A failed request (an HTTP error, or "status" not true) is retried after
 * IAM_RETRY_MS, the wait doubling on each further failure up to IAM_RETRY_MAX_MS.
 * A new connection cancels the wait and is announced at once.
 *
 * Runs in the network task, which it blocks for the length of the request.
 */
class Iam {
	public:
	void begin();   // call once; Network::begin() does in Wi-Fi mode
	void loop();

	private:
	void apply_state(JsonObjectConst state);   // {"<device id>": {"on_off": true, ...}, ...}

	bool pending = false;           // the server has not yet accepted /iam for this connection
	unsigned long last_try_ms = 0;
	unsigned long wait_ms = 0;      // before the next try, 0 = at once
};

}
