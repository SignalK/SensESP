#include "discovery.h"

#include "sensesp.h"
#include "sensesp_app.h"

#include <ESPmDNS.h>

namespace sensesp {

void MDNSDiscovery::start() {
  const char* hostname =
      SensESPBaseApp::get()->get_hostname_observable()->get().c_str();

  // MDNS.begin(hostname) will crash if hostname is blank
  if ((hostname == NULL) || (hostname[0] == '\0')) {
    debugE("hostname has not been set - mDNS can't start");
    return;
  }
  if (!MDNS.begin(hostname)) {  // Start the mDNS responder for hostname.local
    debugW("Error setting up mDNS responder");
  } else {
    debugI("mDNS responder started at %s", hostname);
  }
  mdns_instance_name_set(hostname);  // mDNS hostname for ESP32
  MDNS.addService("http", "tcp", 80);
  MDNS.addService("signalk-sensesp", "tcp", 80);
}

}  // namespace sensesp
