#pragma once

#include <Arduino.h>

#include <vector>

namespace net {

struct HttpResponse {
	int status = 0;     // HTTP status code (>0), or a negative error code
	String body;

	bool ok() const { return status >= 200 && status < 300; }
};

/**
 * @brief Minimal HTTP/HTTPS client. Picks TLS automatically for https:// URLs.
 *
 * HTTPS verifies the server certificate automatically against the built-in
 * Mozilla root-CA bundle (like a browser) — no certificates to ship. Call
 * set_ca_cert() to instead pin one specific root CA (PEM).
 */
class Http {
	public:
	HttpResponse get(const String& url);
	HttpResponse post(const String& url, const String& body, const String& content_type = "application/json");

	void set_header(const String& name, const String& value);   // added to every request
	void clear_headers();
	void set_ca_cert(const char* pem);                          // pin one root CA; nullptr = use bundle
	void set_timeout(uint16_t ms);

	private:
	struct Header {
		String name;
		String value;
	};

	HttpResponse request(const char* method, const String& url, const String* body, const String& content_type);

	std::vector<Header> headers;
	const char* ca_cert = nullptr;
	uint16_t timeout_ms = 10000;
};

}
