/**
 * @file ssl_connection.cpp
 * @brief Example demonstrating SSL/TLS connection to a Signal K server.
 *
 * This example shows how to programmatically enable SSL/TLS with TOFU
 * (Trust On First Use) certificate verification when connecting to a
 * Signal K server that uses a self-signed certificate.
 *
 * IMPORTANT: SSL support requires the pioarduino or espidf platform.
 * It is NOT available with the plain Arduino (espressif32) platform.
 * Your platformio.ini must include:
 *
 *   platform = https://github.com/pioarduino/platform-espressif32/...
 *   build_flags = -D SENSESP_SSL_SUPPORT=1
 *   lib_deps = esp_websocket_client=https://components.espressif.com/...
 *
 * See the SensESP platformio.ini [pioarduino] section for complete settings.
 *
 * HOW TOFU WORKS:
 * - On first connection, the server's certificate fingerprint is captured
 *   and stored in non-volatile memory.
 * - On subsequent connections, the certificate is verified against the
 *   stored fingerprint.
 * - If the certificate changes (e.g., server regenerated its cert), the
 *   connection will fail until you reset TOFU via the web UI or API.
 *
 * This approach provides protection against man-in-the-middle attacks
 * after the initial connection, without requiring a CA certificate bundle.
 */

#include "sensesp/sensors/analog_input.h"
#include "sensesp/signalk/signalk_output.h"
#include "sensesp_app.h"
#include "sensesp_app_builder.h"

using namespace sensesp;

void setup() {
  SetupLogging();

  // Build the SensESP application as usual.
  SensESPAppBuilder builder;
  sensesp_app = builder.set_hostname("ssl-example")->get_app();

  // Get the WebSocket client to configure SSL settings.
  auto ws_client = sensesp_app->get_ws_client();

  // Enable SSL/TLS for the Signal K connection.
  // When enabled, the client will use wss:// (WebSocket Secure) instead of
  // ws://, and https:// instead of http:// for API requests.
  //
  // NOTE: If your Signal K server advertises via mDNS with the "signalk-wss"
  // service type, or if it redirects HTTP to HTTPS, SSL will be enabled
  // automatically. This manual setting is useful when:
  // - You want to force SSL regardless of server advertisement
  // - The server doesn't advertise via mDNS
  // - You're specifying the server address manually
  ws_client->set_ssl_enabled(true);

  // Enable TOFU (Trust On First Use) certificate verification.
  // This is enabled by default, but shown here explicitly for clarity.
  //
  // When enabled:
  // - First connection: Accept any certificate, store its SHA256 fingerprint
  // - Later connections: Verify certificate matches stored fingerprint
  //
  // When disabled:
  // - All certificates are accepted without verification (less secure)
  //
  // To reset a stored fingerprint (e.g., after server cert renewal):
  // - Use the web UI: Settings -> Signal K -> SSL/TLS Security -> Reset TOFU
  // - Or call: ws_client->reset_tofu_fingerprint()
  ws_client->set_tofu_enabled(true);

  // Create a simple sensor to demonstrate the connection is working.
  // This reads the ESP32's internal hall effect sensor and sends it to
  // the Signal K server.
  uint8_t pin = 34;
  auto* analog_input = new AnalogInput(pin, 1000, "/sensors/analog");

  analog_input->connect_to(
      new SKOutputFloat("electrical.batteries.house.voltage"));
}

void loop() { event_loop()->tick(); }
