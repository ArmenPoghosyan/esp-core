#include "network/iam.h"

#include <ArduinoJson.h>

#include <algorithm>

#include "device/manager.h"
#include "env.h"
#include "network/http.h"
#include "network/mqtt.h"
#include "network/network.h"
#include "network/wifi.h"

namespace net {

	void Iam::begin() {
		// Fires in the network task, like loop(), so no locking is needed.
		network.wifi().on_connected([this]() {
			pending = true;
			wait_ms = 0;
			});
	}

	void Iam::loop() {
		if (!pending || !network.wifi().is_connected() || millis() - last_try_ms < wait_ms) {
			return;
		}

		const HttpResponse response = network.http().post(SERVER_URL "/iam", device_manager.to_json());

		JsonDocument doc;
		const bool accepted = response.ok()
			&& deserializeJson(doc, response.body) == DeserializationError::Ok
			&& doc["status"] == true;

		if (accepted) {
			log_i("POST /iam -> %d", response.status);
			pending = false;
			device_manager.apply_state(doc["state"].as<JsonObjectConst>());
			network.mqtt().connect(doc["channel"] | "");
			return;
		}

		last_try_ms = millis();
		wait_ms = wait_ms ? std::min<unsigned long>(wait_ms * 2, IAM_RETRY_MAX_MS) : IAM_RETRY_MS;
		log_e("POST /iam -> %d %s, retry in %lu ms", response.status, response.body.c_str(), wait_ms);
	}

}
