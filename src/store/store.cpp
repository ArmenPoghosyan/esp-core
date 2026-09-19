#include "store/store.h"

#include "env.h"   // MEMORY_TYPE + MEMORY_* option macros (via consts.h)

// An undefined name counts as 0 inside #if, so a missing include would
// silently select the EEPROM backend. Fail loudly instead.
#if !defined(MEMORY_TYPE) || !defined(MEMORY_PREFERENCES)
#error "MEMORY_TYPE is not defined: it comes from env.h and consts.h"
#endif

// Pull in the backend header for the selected memory type.
#if MEMORY_TYPE == MEMORY_EEPROM
#include <EEPROM.h>
#include <vector>
#include <cstdlib>
#elif MEMORY_TYPE == MEMORY_FLASH
#include <LittleFS.h>
#include <vector>
#include <cstdlib>
#else
#include <Preferences.h>
#endif

// The constructor is backend-agnostic; it just remembers the namespace.
Store::Store(const char* ns) : ns(ns) {}

// ===========================================================================
//  EEPROM and FLASH share the same emulation: the whole store is a list of
//  (namespace, key, value) records serialized into one blob. Only *where* the
//  blob is persisted differs, which is the backend_read()/backend_write() hook.
// ===========================================================================
#if MEMORY_TYPE == MEMORY_EEPROM || MEMORY_TYPE == MEMORY_FLASH

namespace {
	struct Record {
		String ns;
		String key;
		String value;
	};

	// Persistence hooks — defined in the backend-specific section further down.
	std::vector<uint8_t> backend_read();
	void backend_write(const std::vector<uint8_t>& blob);

	void write_u16(std::vector<uint8_t>& out, uint16_t value) {
		out.push_back(static_cast<uint8_t>(value >> 8));
		out.push_back(static_cast<uint8_t>(value & 0xFF));
	}

	void write_str(std::vector<uint8_t>& out, const String& s) {
		write_u16(out, static_cast<uint16_t>(s.length()));
		for (size_t i = 0; i < s.length(); i++) {
			out.push_back(static_cast<uint8_t>(s[i]));
		}
	}

	uint16_t read_u16(const std::vector<uint8_t>& in, size_t& pos) {
		if (pos + 1 >= in.size()) {
			pos = in.size();
			return 0;
		}
		const uint16_t value = (static_cast<uint16_t>(in[pos]) << 8) | in[pos + 1];
		pos += 2;
		return value;
	}

	String read_str(const std::vector<uint8_t>& in, size_t& pos) {
		const uint16_t len = read_u16(in, pos);
		String s;
		s.reserve(len);
		for (uint16_t i = 0; i < len && pos < in.size(); i++) {
			s += static_cast<char>(in[pos++]);
		}
		return s;
	}

	std::vector<Record> load_all() {
		std::vector<Record> records;

		const std::vector<uint8_t> blob = backend_read();
		if (blob.size() < 2) {
			return records;   // empty / uninitialized
		}

		size_t pos = 0;
		const uint16_t count = read_u16(blob, pos);
		for (uint16_t i = 0; i < count && pos < blob.size(); i++) {
			Record r;
			r.ns = read_str(blob, pos);
			r.key = read_str(blob, pos);
			r.value = read_str(blob, pos);
			records.push_back(r);
		}

		return records;
	}

	void save_all(const std::vector<Record>& records) {
		std::vector<uint8_t> blob;
		write_u16(blob, static_cast<uint16_t>(records.size()));
		for (const Record& r : records) {
			write_str(blob, r.ns);
			write_str(blob, r.key);
			write_str(blob, r.value);
		}
		backend_write(blob);
	}
}

void Store::clear() {
	std::vector<Record> kept;
	for (const Record& r : load_all()) {
		if (r.ns != ns) {
			kept.push_back(r);   // keep other namespaces, drop ours
		}
	}
	save_all(kept);
}

bool Store::has_key(const char* key) const {
	for (const Record& r : load_all()) {
		if (r.ns == ns && r.key == key) {
			return true;
		}
	}
	return false;
}

void Store::remove_key(const char* key) {
	std::vector<Record> records = load_all();
	for (size_t i = 0; i < records.size(); i++) {
		if (records[i].ns == ns && records[i].key == key) {
			records.erase(records.begin() + i);
			save_all(records);
			return;
		}
	}
}

String Store::get_string(const char* key, const String& fallback) const {
	for (const Record& r : load_all()) {
		if (r.ns == ns && r.key == key) {
			return r.value;
		}
	}
	return fallback;
}

void Store::put_string(const char* key, const String& value) {
	std::vector<Record> records = load_all();
	for (Record& r : records) {
		if (r.ns == ns && r.key == key) {
			r.value = value;
			save_all(records);
			return;
		}
	}
	records.push_back(Record{ String(ns), String(key), value });
	save_all(records);
}

uint32_t Store::get_uint(const char* key, uint32_t fallback) const {
	for (const Record& r : load_all()) {
		if (r.ns == ns && r.key == key) {
			return static_cast<uint32_t>(strtoul(r.value.c_str(), nullptr, 10));
		}
	}
	return fallback;
}

void Store::put_uint(const char* key, uint32_t value) {
	put_string(key, String(value));
}

#endif   // shared EEPROM/FLASH store logic

// ===========================================================================
//  Backend-specific persistence.
// ===========================================================================
#if MEMORY_TYPE == MEMORY_EEPROM

// One flat EEPROM region holds the whole serialized store: [uint16 length][blob].
#ifndef STORE_EEPROM_SIZE
#define STORE_EEPROM_SIZE 512
#endif

namespace {
	bool eeprom_ready = false;

	void eeprom_ensure() {
		if (!eeprom_ready) {
			EEPROM.begin(STORE_EEPROM_SIZE);
			eeprom_ready = true;
		}
	}

	std::vector<uint8_t> backend_read() {
		eeprom_ensure();

		std::vector<uint8_t> blob;
		const uint16_t length = (static_cast<uint16_t>(EEPROM.read(0)) << 8) | EEPROM.read(1);
		if (length == 0xFFFF || static_cast<size_t>(length) + 2 > STORE_EEPROM_SIZE) {
			return blob;   // uninitialized flash or corrupt length
		}

		blob.reserve(length);
		for (uint16_t i = 0; i < length; i++) {
			blob.push_back(EEPROM.read(2 + i));
		}
		return blob;
	}

	void backend_write(const std::vector<uint8_t>& blob) {
		eeprom_ensure();

		if (blob.size() + 2 > STORE_EEPROM_SIZE) {
			return;   // would overflow the region — refuse rather than corrupt
		}

		EEPROM.write(0, static_cast<uint8_t>(blob.size() >> 8));
		EEPROM.write(1, static_cast<uint8_t>(blob.size() & 0xFF));
		for (size_t i = 0; i < blob.size(); i++) {
			EEPROM.write(2 + i, blob[i]);
		}
		EEPROM.commit();
	}
}

#elif MEMORY_TYPE == MEMORY_FLASH

// The whole serialized store lives in a single LittleFS file.
namespace {
	constexpr const char* STORE_PATH = "/store.kv";
	bool fs_ready = false;

	void fs_ensure() {
		if (!fs_ready) {
			LittleFS.begin(/*formatOnFail=*/true);
			fs_ready = true;
		}
	}

	std::vector<uint8_t> backend_read() {
		fs_ensure();

		std::vector<uint8_t> blob;
		File file = LittleFS.open(STORE_PATH, "r");
		if (!file) {
			return blob;
		}

		blob.reserve(file.size());
		while (file.available()) {
			blob.push_back(static_cast<uint8_t>(file.read()));
		}
		file.close();
		return blob;
	}

	void backend_write(const std::vector<uint8_t>& blob) {
		fs_ensure();

		File file = LittleFS.open(STORE_PATH, "w");
		if (!file) {
			return;
		}

		if (!blob.empty()) {
			file.write(blob.data(), blob.size());
		}
		file.close();
	}
}

#else   // ---- MEMORY_PREFERENCES (ESP32, default) ----

void Store::clear() {
	Preferences prefs;
	prefs.begin(ns, /*readOnly=*/false);
	prefs.clear();
	prefs.end();
}

bool Store::has_key(const char* key) const {
	Preferences prefs;
	prefs.begin(ns, /*readOnly=*/true);
	const bool present = prefs.isKey(key);
	prefs.end();
	return present;
}

void Store::remove_key(const char* key) {
	Preferences prefs;
	prefs.begin(ns, /*readOnly=*/false);
	prefs.remove(key);
	prefs.end();
}

String Store::get_string(const char* key, const String& fallback) const {
	Preferences prefs;
	prefs.begin(ns, /*readOnly=*/true);
	String value = prefs.getString(key, fallback);
	prefs.end();
	return value;
}

void Store::put_string(const char* key, const String& value) {
	Preferences prefs;
	prefs.begin(ns, /*readOnly=*/false);
	prefs.putString(key, value);
	prefs.end();
}

uint32_t Store::get_uint(const char* key, uint32_t fallback) const {
	Preferences prefs;
	prefs.begin(ns, /*readOnly=*/true);
	const uint32_t value = prefs.getUInt(key, fallback);
	prefs.end();
	return value;
}

void Store::put_uint(const char* key, uint32_t value) {
	Preferences prefs;
	prefs.begin(ns, /*readOnly=*/false);
	prefs.putUInt(key, value);
	prefs.end();
}

#endif
