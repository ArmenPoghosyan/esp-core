#pragma once

#include "consts.h"

#define DEVICE_MODE					DEVICE_MODE_WIFI

// ? Memory Type
#define MEMORY_TYPE					MEMORY_PREFERENCES

// ? WiFi configuration
#define WIFI_LED_PIN				LED_BUILTIN		// status LED
#define WIFI_BUTTON_PIN				0				// mode button
#define WIFI_BUTTON_HOLD_MS			5000			// hold to toggle captive mode
#define WIFI_BLINK_IDLE_MS			1000			// blink: not connected
#define WIFI_BLINK_CAPTIVE_MS		125				// blink: captive mode
#define WIFI_CONNECT_TIMEOUT_MS		15000			// upper bound per credential (absent networks are skipped much sooner)
#define WIFI_CONNECT_ATTEMPTS		2				// tries per credential when the AP answers but the join fails
#define WIFI_PREFER_LAST_GOOD		1				// 1: start with the network that worked last, 0: always primary -> backups
#define WIFI_RETRY_MS				10000			// wait before retrying after all fail
#define WIFI_AP_PREFIX				DEVICE_MANUFACTURER	// AP + LAN hostname prefix (a MAC suffix is appended)

#define NETWORK_TASK_STACK			8192			// bytes; the setup portal's web server needs most of it
#define NETWORK_TASK_PRIORITY		1				// same as the Arduino loop task
#define NETWORK_LOOP_MS				10				// period of Network::loop()

// ? Server
#define SERVER_URL					"https://iot.aplink.am"
#define IAM_RETRY_MS				5000			// wait after a failed /iam; doubles on each further failure
#define IAM_RETRY_MAX_MS			300000			// upper bound for that wait

#define MQTT_HOST					"iot.aplink.am"
#define MQTT_PORT					8883			// TLS
#define MQTT_USERNAME				"aplink_iot"
#define MQTT_PASSWORD				"7094a64deda36a66e72f888a6f98a081"
#define MQTT_BUFFER_SIZE			1024			// bytes; the largest message, topic included
#define MQTT_TIMEOUT_S				10				// TCP connect and TLS handshake, each; blocks the network task
#define MQTT_RETRY_MS				5000			// wait after a failed broker connect; doubles on each further failure
#define MQTT_RETRY_MAX_MS			300000			// upper bound for that wait

// ? Device information
#define DEVICE_ID_LENGTH			18
#define DEVICE_NAME					"APLink Controller"
#define DEVICE_MANUFACTURER			"APLink"
#define DEVICE_HARDWARE_VERSION		"1.0.0"
#define DEVICE_SOFTWARE_VERSION		"1.0.0"
