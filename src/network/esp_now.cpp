#include "network/esp_now.h"

#include <WiFi.h>
#include <esp_wifi.h>

#include <string.h>

namespace net {

	namespace {
		// Per-fragment header prepended to every ESP-NOW frame.
		struct __attribute__((packed)) FragHeader {
			uint32_t msg_id;
			uint16_t total_len;   // whole message size
			uint16_t index;       // this fragment's index
			uint16_t count;       // total fragments
			uint16_t len;         // this fragment's payload size
		};

		constexpr size_t MAX_PAYLOAD = ESP_NOW_MAX_DATA_LEN - sizeof(FragHeader);   // 238 bytes
		constexpr unsigned long SEND_TIMEOUT_MS = 50;    // no ACK within this = failed fragment
		constexpr uint8_t MAX_RETRIES = 5;
		constexpr unsigned long REASM_TIMEOUT_MS = 5000; // drop half-received messages after this

		constexpr int STATUS_PENDING = 0;
		constexpr int STATUS_SUCCESS = 1;
		constexpr int STATUS_FAIL = 2;

		// One completed message passed from the WiFi task to loop().
		struct Delivered {
			uint8_t mac[6];
			std::vector<uint8_t> data;
		};

		EspNow* s_self = nullptr;

		uint64_t mac_to_key(const uint8_t* mac) {
			uint64_t key = 0;
			for (int i = 0; i < 6; i++) {
				key = (key << 8) | mac[i];
			}
			return key;
		}
	}

	void EspNow::begin() {
		if (delivered) {
			return;   // already started
		}

		s_self = this;
		delivered = xQueueCreate(8, sizeof(Delivered*));

		// ESP-NOW needs Wi-Fi started; leave an existing mode (set by the Wifi module) alone.
		if (WiFi.getMode() == WIFI_MODE_NULL) {
			WiFi.mode(WIFI_STA);
		}

		esp_now_init();
		esp_now_register_send_cb(EspNow::on_send);
		esp_now_register_recv_cb(EspNow::on_recv);
	}

	void EspNow::loop() {
		// Deliver completed messages in the main task context.
		Delivered* item = nullptr;
		while (delivered && xQueueReceive(delivered, &item, 0) == pdTRUE) {
			if (handler) {
				handler(item->mac, item->data.data(), item->data.size());
			}
			delete item;
		}

		// Drive the outgoing transfer at the front of the queue (stop-and-wait).
		if (outgoing.empty()) {
			return;
		}
		OutTransfer& t = outgoing.front();

		if (!t.in_flight) {
			send_fragment(t);
			return;
		}

		// Waiting for the send callback (or a timeout).
		if (send_status == STATUS_PENDING) {
			if (millis() - t.sent_ms < SEND_TIMEOUT_MS) {
				return;
			}
			send_status = STATUS_FAIL;   // no ACK in time
		}

		if (send_status == STATUS_SUCCESS) {
			t.in_flight = false;
			t.retries = 0;
			t.index++;
			if (t.index >= t.count) {
				outgoing.erase(outgoing.begin());   // whole message delivered
			}
		} else {   // STATUS_FAIL
			t.in_flight = false;
			if (++t.retries > MAX_RETRIES) {
				outgoing.erase(outgoing.begin());   // give up on this message
			}
			// otherwise the same fragment is resent next loop
		}
	}

	void EspNow::on_message(Handler h) {
		handler = std::move(h);
	}

	bool EspNow::add_peer(const uint8_t mac[6]) {
		ensure_peer(mac);
		return esp_now_is_peer_exist(mac);
	}

	bool EspNow::send(const uint8_t mac[6], const uint8_t* data, size_t len) {
		if (len == 0 || len > 65535) {
			return false;
		}

		OutTransfer t;
		memcpy(t.mac, mac, 6);
		t.msg_id = next_msg_id++;
		t.data.assign(data, data + len);
		t.count = static_cast<uint16_t>((len + MAX_PAYLOAD - 1) / MAX_PAYLOAD);
		t.index = 0;
		t.retries = 0;
		t.in_flight = false;
		t.sent_ms = 0;

		outgoing.push_back(std::move(t));
		return true;
	}

	bool EspNow::send(const uint8_t mac[6], const String& data) {
		return send(mac, reinterpret_cast<const uint8_t*>(data.c_str()), data.length());
	}

	void EspNow::ensure_peer(const uint8_t mac[6]) {
		if (esp_now_is_peer_exist(mac)) {
			return;
		}

		esp_now_peer_info_t peer = {};
		memcpy(peer.peer_addr, mac, 6);
		peer.channel = 0;              // use the current Wi-Fi channel
		peer.ifidx = WIFI_IF_STA;
		peer.encrypt = false;
		esp_now_add_peer(&peer);
	}

	void EspNow::send_fragment(OutTransfer& t) {
		const uint16_t offset = t.index * MAX_PAYLOAD;
		const uint16_t len = static_cast<uint16_t>(min<size_t>(MAX_PAYLOAD, t.data.size() - offset));

		FragHeader header;
		header.msg_id = t.msg_id;
		header.total_len = static_cast<uint16_t>(t.data.size());
		header.index = t.index;
		header.count = t.count;
		header.len = len;

		uint8_t frame[ESP_NOW_MAX_DATA_LEN];
		memcpy(frame, &header, sizeof(header));
		memcpy(frame + sizeof(header), t.data.data() + offset, len);

		ensure_peer(t.mac);

		send_status = STATUS_PENDING;
		t.in_flight = true;
		t.sent_ms = millis();

		if (esp_now_send(t.mac, frame, sizeof(header) + len) != ESP_OK) {
			send_status = STATUS_FAIL;   // couldn't even queue it
		}
	}

	void EspNow::on_send(const uint8_t* /*mac*/, esp_now_send_status_t status) {
		if (s_self) {
			s_self->send_status = (status == ESP_NOW_SEND_SUCCESS) ? STATUS_SUCCESS : STATUS_FAIL;
		}
	}

	void EspNow::on_recv(const uint8_t* mac, const uint8_t* data, int len) {
		if (s_self && len >= static_cast<int>(sizeof(FragHeader))) {
			s_self->handle_fragment(mac, data, len);
		}
	}

	void EspNow::handle_fragment(const uint8_t* mac, const uint8_t* data, int len) {
		FragHeader header;
		memcpy(&header, data, sizeof(header));

		if (header.len > len - static_cast<int>(sizeof(FragHeader)) || header.count == 0) {
			return;   // malformed
		}

		const uint64_t key = mac_to_key(mac);
		Reasm& r = incoming[key][header.msg_id];

		if (r.data.empty() && r.got.empty()) {
			r.data.assign(header.total_len, 0);
			r.got.assign(header.count, false);
			r.remaining = header.count;
		}

		const uint16_t offset = header.index * MAX_PAYLOAD;
		if (header.index < r.got.size() && !r.got[header.index] &&
			offset + header.len <= r.data.size()) {
			memcpy(r.data.data() + offset, data + sizeof(FragHeader), header.len);
			r.got[header.index] = true;
			r.remaining--;
		}
		r.last_ms = millis();

		if (r.remaining == 0) {
			Delivered* item = new Delivered();
			memcpy(item->mac, mac, 6);
			item->data = std::move(r.data);

			incoming[key].erase(header.msg_id);
			if (incoming[key].empty()) {
				incoming.erase(key);
			}

			if (xQueueSend(delivered, &item, 0) != pdTRUE) {
				delete item;   // queue full -> drop
			}
		}

		// Drop any half-received messages that have gone stale.
		const unsigned long now = millis();
		for (auto peer = incoming.begin(); peer != incoming.end();) {
			for (auto msg = peer->second.begin(); msg != peer->second.end();) {
				if (now - msg->second.last_ms > REASM_TIMEOUT_MS) {
					msg = peer->second.erase(msg);
				} else {
					++msg;
				}
			}
			peer = peer->second.empty() ? incoming.erase(peer) : std::next(peer);
		}
	}

}
