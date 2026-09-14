#pragma once

#include "device/consts.h"

// ? Memory Type
#define MEMORY_TYPE					MEMORY_PREFERENCES

// ? WiFi configuration
#define WIFI_LED_PIN				LED_BUILTIN		// status LED
#define WIFI_BUTTON_PIN				0				// mode button
#define WIFI_BUTTON_HOLD_MS			5000			// hold to toggle captive mode
#define WIFI_BLINK_IDLE_MS			1000			// blink: not connected
#define WIFI_BLINK_CAPTIVE_MS		125				// blink: captive mode
#define WIFI_CONNECT_TIMEOUT_MS		15000			// per-credential connect attempt
#define WIFI_RETRY_MS				10000			// wait before retrying after all fail
#define WIFI_AP_PREFIX				DEVICE_MANUFACTURER	// AP + LAN hostname prefix (a MAC suffix is appended)
#define WIFI_NTP_SERVER				"pool.ntp.org"		// time sync (needed for TLS cert validation)

// ? Device information
#define DEVICE_NAME					"Light"
#define DEVICE_MANUFACTURER			"APSuite"
#define DEVICE_FIRMWARE_VERSION		"1.0.0"
