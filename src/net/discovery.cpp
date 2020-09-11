#include "sensesp.h"

#ifdef ESP8266
  #include <ESP8266mDNS.h>        // Include the mDNS library
#elif defined(ESP32)
  #include <ESPmDNS.h>
#endif


void setup_discovery(const char* hostname) {
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
  MDNS.addService("http", "tcp", 80);
  MDNS.addService("signalk-sensesp", "tcp", 80);
}
