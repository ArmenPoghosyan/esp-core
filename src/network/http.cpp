#include "network/http.h"

#include <HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>

// Full Mozilla root-CA bundle embedded in the ESP-IDF mbedTLS build
// (CONFIG_MBEDTLS_CERTIFICATE_BUNDLE_DEFAULT_FULL). Lets us verify any standard
// HTTPS server automatically, with no certificate files to ship.
extern const uint8_t ca_cert_bundle[] asm("x509_crt_bundle");

namespace net {

HttpResponse Http::get(const String& url) {
	return request("GET", url, nullptr, "");
}

HttpResponse Http::post(const String& url, const String& body, const String& content_type) {
	return request("POST", url, &body, content_type);
}

void Http::set_header(const String& name, const String& value) {
	headers.push_back(Header{name, value});
}

void Http::clear_headers() {
	headers.clear();
}

void Http::set_ca_cert(const char* pem) {
	ca_cert = pem;
}

void Http::set_timeout(uint16_t ms) {
	timeout_ms = ms;
}

HttpResponse Http::request(const char* method, const String& url, const String* body, const String& content_type) {
	HttpResponse response;

	// These clients must outlive the request; HTTPClient keeps a reference.
	WiFiClientSecure secure;
	WiFiClient plain;
	HTTPClient http;

	bool began;
	if (url.startsWith("https:")) {
		if (ca_cert) {
			secure.setCACert(ca_cert);                  // pinned single root CA
		} else {
			secure.setCACertBundle(ca_cert_bundle);     // verify against Mozilla roots
		}
		began = http.begin(secure, url);
	} else {
		began = http.begin(plain, url);
	}

	if (!began) {
		response.status = -1;   // malformed URL / could not init
		return response;
	}

	http.setConnectTimeout(timeout_ms);
	http.setTimeout(timeout_ms);

	for (const Header& header : headers) {
		http.addHeader(header.name, header.value);
	}

	if (body) {
		http.addHeader("Content-Type", content_type);
		response.status = http.POST(*body);
	} else {
		response.status = http.GET();
	}

	if (response.status > 0) {
		response.body = http.getString();
	}

	http.end();
	return response;
}

}
