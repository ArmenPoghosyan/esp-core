#pragma once

#include "store/store.h"

#include <vector>

struct Credentials {
	String ssid;
	String password;
};

/**
 * @brief Stores one or more Wi-Fi credentials (primary + backups) in NVS.
 *
 * Credentials are an ordered list: index 0 is the primary network and later
 * entries are fallbacks (backup Wi-Fi). Order is preserved across add/remove.
 */
class CredentialsStore : public Store {
	public:
	CredentialsStore();

	size_t count() const;
	std::vector<Credentials> get_credentials() const;
	bool get_credentials(size_t index, Credentials& out) const;

	void add_credentials(const String& ssid, const String& password);
	bool remove_credentials(size_t index);
	void clear_credentials();
};
