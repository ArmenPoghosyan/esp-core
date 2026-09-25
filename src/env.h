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
#define WIFI_NTP_SERVER				"pool.ntp.org"		// time sync (needed for TLS cert validation)

// ? Device information
#define DEVICE_MANUFACTURER			"APLink"
#define DEVICE_FIRMWARE_VERSION		"1.0.0"
#define DEVICE_SOFTWARE_VERSION		"1.0.0"
