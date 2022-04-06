#include "discovery.h"

#include "sensesp.h"
#include "sensesp_app.h"

#include <ESPmDNS.h>

namespace sensesp {

void MDNSDiscovery::start() {
  String hostname = SensESPBaseApp::get_hostname();

  // MDNS.begin(hostname) will crash if hostname is blank
  if ((hostname == "")) {
    debugE("hostname has not been set - mDNS can't start");
    return;
  }
  if (!MDNS.begin(hostname.c_str())) {  // Start the mDNS responder for hostname.local
    debugW("Error setting up mDNS responder");
  } else {
    debugI("mDNS responder started for hostname '%s'", hostname.c_str());
  }
  mdns_instance_name_set(hostname.c_str());  // mDNS hostname for ESP32
}

}  // namespace sensesp
