#pragma once

#include <Arduino.h>

/**
 * @brief Base class for a namespaced key/value store.
 *
 * The persistence backend (Preferences on ESP32, EEPROM on ESP8266/AVR, a flash
 * filesystem, ...) is selected at compile time from MEMORY_TYPE in env.h and
 * implemented entirely in store.cpp behind #if blocks. Subclasses (e.g.
 * CredentialsStore) use the protected helpers and never see the backend.
 */
class Store {
	public:
	explicit Store(const char* ns);
	virtual ~Store() = default;

	/**
	 * @brief Erase every key stored in this namespace.
	 */
	void clear();

	protected:
	bool has_key(const char* key) const;
	void remove_key(const char* key);

	String get_string(const char* key, const String& fallback = String()) const;
	void put_string(const char* key, const String& value);

	uint32_t get_uint(const char* key, uint32_t fallback = 0) const;
	void put_uint(const char* key, uint32_t value);

	private:
	const char* ns;
};
