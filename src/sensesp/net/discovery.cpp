#include "discovery.h"

#include "sensesp_base_app.h"

namespace sensesp {

MDNSDiscovery::MDNSDiscovery() {
  SensESPBaseApp::get_event_loop()->onDelay(0, [this]() {
    String hostname = SensESPBaseApp::get_hostname();

    // MDNS.begin(hostname) will crash if hostname is blank
    if ((hostname == "")) {
      ESP_LOGE(__FILENAME__, "hostname has not been set - mDNS can't start");
      return;
    }
    if (!MDNS.begin(
            hostname.c_str())) {  // Start the mDNS responder for hostname.local
      ESP_LOGW(__FILENAME__, "Error setting up mDNS responder");
    } else {
      ESP_LOGI(__FILENAME__, "mDNS responder started for hostname '%s'",
               hostname.c_str());
    }
    mdns_instance_name_set(hostname.c_str());  // mDNS hostname for ESP32
  });
}

}  // namespace sensesp
