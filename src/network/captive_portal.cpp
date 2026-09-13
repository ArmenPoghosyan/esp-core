#include "network/captive_portal.h"

#include <WiFi.h>

#include "network/dns.h"
#include "network/network.h"

namespace net {

void CaptivePortal::begin() {
	if (running) {
		return;
	}

	network.dns().start(WiFi.softAPIP());

	server.on("/", [this]() { handle_root(); });
	server.on("/add", HTTP_POST, [this]() { handle_add(); });
	server.on("/remove", [this]() { handle_remove(); });
	server.on("/connect", HTTP_POST, [this]() { handle_finish(); });

	// Any other URL (incl. OS connectivity probes like /generate_204,
	// /hotspot-detect.html) gets redirected to the portal, which is what makes
	// phones show "Sign in to network" and auto-open the page.
	server.onNotFound([this]() { redirect_to_portal(); });
	server.begin();

	running = true;
}

void CaptivePortal::loop() {
	if (running) {
		server.handleClient();
	}
}

void CaptivePortal::stop() {
	if (!running) {
		return;
	}

	server.stop();
	network.dns().stop();
	running = false;
}

bool CaptivePortal::is_running() const {
	return running;
}

void CaptivePortal::on_finished(std::function<void()> callback) {
	finished_cb = std::move(callback);
}

void CaptivePortal::handle_root() {
	String html = F(
		"<!doctype html><meta charset=utf-8>"
		"<meta name=viewport content='width=device-width,initial-scale=1'>"
		"<title>Wi-Fi Setup</title><style>"
		"body{font-family:system-ui,sans-serif;max-width:360px;margin:24px auto;padding:0 16px;color:#222}"
		"h1{font-size:20px}h2{font-size:15px;color:#666;margin:20px 0 6px}"
		"ul{list-style:none;padding:0;margin:0}"
		"li{display:flex;justify-content:space-between;align-items:center;padding:8px 0;border-bottom:1px solid #eee}"
		"select,input,button{width:100%;padding:10px;margin:4px 0;box-sizing:border-box;border:1px solid #ccc;border-radius:8px;font-size:14px}"
		"button{background:#0a84ff;color:#fff;border:0;font-weight:600}"
		"a{color:#e00;text-decoration:none;font-size:13px}"
		"</style><h1>Wi-Fi Setup</h1>");

	const std::vector<Credentials> saved = credentials.get_credentials();
	if (!saved.empty()) {
		html += F("<h2>SAVED NETWORKS</h2><ul>");
		for (size_t i = 0; i < saved.size(); i++) {
			html += "<li><span>" + saved[i].ssid + "</span>"
				"<a href='/remove?i=" + String(i) + "'>remove</a></li>";
		}
		html += F("</ul>");
	}

	html += F("<h2>ADD NETWORK</h2><form method=POST action=/add>"
		"<input name=ssid placeholder='Network name'>"
		"<input name=pass type=password placeholder=Password>"
		"<button>Add</button></form>"
		"<form method=POST action=/connect><button>Done &amp; Connect</button></form>");

	server.send(200, "text/html", html);
}

void CaptivePortal::handle_add() {
	const String ssid = server.arg("ssid");
	const String pass = server.arg("pass");
	if (!ssid.isEmpty()) {
		credentials.add_credentials(ssid, pass);
	}
	redirect_home();
}

void CaptivePortal::handle_remove() {
	if (server.hasArg("i")) {
		credentials.remove_credentials(server.arg("i").toInt());
	}
	redirect_home();
}

void CaptivePortal::handle_finish() {
	server.send(200, "text/html", F(
		"<!doctype html><meta name=viewport content='width=device-width,initial-scale=1'>"
		"<body style='font-family:system-ui,sans-serif;text-align:center;margin-top:48px'>"
		"<h2>Connecting&hellip;</h2><p>You can close this page.</p>"));

	if (finished_cb) {
		finished_cb();
	}
}

void CaptivePortal::redirect_home() {
	server.sendHeader("Location", "/");
	server.send(303);   // See Other -> GET "/"
}

void CaptivePortal::redirect_to_portal() {
	// Absolute URL to the AP so probes (sent with a foreign Host header) land here.
	server.sendHeader("Location", "http://" + WiFi.softAPIP().toString() + "/");
	server.send(302, "text/plain", "");
}

}
