#include "store/credentials_store.h"

namespace {
	constexpr const char* KEY_COUNT = "count";

	// NVS keys are limited to 15 chars; "ssid"/"pass" + index stays well within.
	String ssid_key(size_t index) {
		char buf[16];
		snprintf(buf, sizeof(buf), "ssid%u", static_cast<unsigned>(index));
		return String(buf);
	}

	String pass_key(size_t index) {
		char buf[16];
		snprintf(buf, sizeof(buf), "pass%u", static_cast<unsigned>(index));
		return String(buf);
	}
}

CredentialsStore::CredentialsStore() : Store("wifi") {}

size_t CredentialsStore::count() const {
	return get_uint(KEY_COUNT, 0);
}

bool CredentialsStore::get_credentials(size_t index, Credentials& out) const {
	if (index >= count()) {
		return false;
	}

	out.ssid = get_string(ssid_key(index).c_str());
	out.password = get_string(pass_key(index).c_str());
	return true;
}

std::vector<Credentials> CredentialsStore::get_credentials() const {
	const size_t n = count();

	std::vector<Credentials> list;
	list.reserve(n);

	for (size_t i = 0; i < n; i++) {
		Credentials c;
		c.ssid = get_string(ssid_key(i).c_str());
		c.password = get_string(pass_key(i).c_str());
		list.push_back(c);
	}

	return list;
}

void CredentialsStore::add_credentials(const String& ssid, const String& password) {
	const size_t index = count();

	put_string(ssid_key(index).c_str(), ssid);
	put_string(pass_key(index).c_str(), password);
	put_uint(KEY_COUNT, index + 1);
}

bool CredentialsStore::remove_credentials(size_t index) {
	const size_t n = count();
	if (index >= n) {
		return false;
	}

	// Shift later entries down so the list stays contiguous and ordered.
	for (size_t i = index; i + 1 < n; i++) {
		put_string(ssid_key(i).c_str(), get_string(ssid_key(i + 1).c_str()));
		put_string(pass_key(i).c_str(), get_string(pass_key(i + 1).c_str()));
	}

	remove_key(ssid_key(n - 1).c_str());
	remove_key(pass_key(n - 1).c_str());
	put_uint(KEY_COUNT, n - 1);
	return true;
}

void CredentialsStore::clear_credentials() {
	clear();
}
