#include "sensesp.h"
#include "sensesp_app.h"

#include "discovery.h"

#ifdef ESP8266
  #include <ESP8266mDNS.h>        // Include the mDNS library
#elif defined(ESP32)
  #include <ESPmDNS.h>
#endif

void MDNSDiscovery::start() {
  const char* hostname = sensesp_app->get_hostname().c_str();

  // MDNS.begin(hostname) will crash if hostname is blank
  if ((hostname == NULL) || (hostname[0] == '\0')) {
   debugE("hostname has not been set - mDNS can't start");
   return;
  }
  if (!MDNS.begin(hostname)) {             // Start the mDNS responder for esp8266.local
    debugW("Error setting up mDNS responder");
  } else {
    debugI("mDNS responder started at %s", hostname);
  }
#ifdef ESP32
  mdns_instance_name_set(hostname);   //mDNS hostname for ESP32
#endif
  MDNS.addService("http", "tcp", 80);
  MDNS.addService("signalk-sensesp", "tcp", 80);
}
