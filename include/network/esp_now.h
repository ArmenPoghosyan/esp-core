#pragma once

#include <Arduino.h>

#include <esp_now.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include <functional>
#include <map>
#include <vector>

namespace net {

	/**
	 * @brief Reliable, ordered messaging over ESP-NOW with no 250-byte limit.
	 *
	 * ESP-NOW frames are capped at 250 bytes, so messages are split into fragments
	 * and reassembled by the receiver. Delivery is confirmed per fragment using the
	 * ESP-NOW send callback (MAC-level ACK) with retries — TCP-like reliability
	 * without a router. A whole message (up to 65535 bytes) arrives intact or not
	 * at all.
	 *
	 * Peers must share the same Wi-Fi channel (e.g. both joined to the same AP).
	 */
	class EspNow {
		public:
		using Handler = std::function<void(const uint8_t mac[6], const uint8_t* data, size_t len)>;

		void begin();
		void loop();

		void on_message(Handler handler);

		bool add_peer(const uint8_t mac[6]);
		bool send(const uint8_t mac[6], const uint8_t* data, size_t len);
		bool send(const uint8_t mac[6], const String& data);

		private:
		struct OutTransfer {
			uint8_t mac[6];
			uint32_t msg_id;
			std::vector<uint8_t> data;
			uint16_t count;
			uint16_t index;
			uint8_t retries;
			bool in_flight;
			unsigned long sent_ms;
		};

		struct Reasm {
			std::vector<uint8_t> data;
			std::vector<bool> got;
			uint16_t remaining;
			unsigned long last_ms;
		};

		static void on_send(const uint8_t* mac, esp_now_send_status_t status);
		static void on_recv(const uint8_t* mac, const uint8_t* data, int len);

		void handle_fragment(const uint8_t* mac, const uint8_t* data, int len);
		void send_fragment(OutTransfer& transfer);
		void ensure_peer(const uint8_t mac[6]);

		Handler handler;
		std::vector<OutTransfer> outgoing;
		std::map<uint64_t, std::map<uint32_t, Reasm>> incoming;   // key: sender MAC -> msg_id
		uint32_t next_msg_id = 1;
		volatile int send_status = 0;   // 0 pending, 1 success, 2 fail
		QueueHandle_t delivered = nullptr;
	};

}
