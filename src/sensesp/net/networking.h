#ifndef SENSESP_NET_NETWORKING_H_
#define SENSESP_NET_NETWORKING_H_

// Backward-compat shim. The previous monolithic Networking class has been
// split into:
//
//   - NetworkProvisioner   abstract base, transport-agnostic IP query API
//                          (network_provisioner.h)
//   - WiFiProvisioner      concrete WiFi implementation, formerly named
//                          Networking (wifi_provisioner.h)
//   - EthernetProvisioner  concrete native-Ethernet implementation
//                          (ethernet_provisioner.h)
//
// And:
//
//   - NetworkState         transport-agnostic state enum, alias of WiFiState
//                          (network_state.h)
//   - NetworkStateProducer single producer that listens to the unified
//                          Arduino-ESP32 Network event bus and emits state
//                          for any active interface (network_state.h)
//
// Existing user code that includes <sensesp/net/networking.h> and writes
// `Networking` (or `WiFiState`) continues to compile because of the
// aliases here plus those in network_state.h.
//
// New code should prefer:
//   #include "sensesp/net/wifi_provisioner.h"     // for WiFiProvisioner
//   #include "sensesp/net/ethernet_provisioner.h" // for EthernetProvisioner
//   #include "sensesp/net/network_state.h"        // for NetworkState
//   #include "sensesp/net/network_provisioner.h"  // for the abstract base

#include "sensesp/net/network_provisioner.h"
#include "sensesp/net/network_state.h"
#include "sensesp/net/wifi_provisioner.h"

namespace sensesp {

/// @deprecated Use WiFiProvisioner directly, or NetworkProvisioner if
/// transport-agnostic. Kept as a typedef so existing user code continues
/// to compile.
using Networking = WiFiProvisioner;

/// @deprecated Use NetworkStateProducer directly. Kept so code that
/// references the WiFiStateProducer type by name continues to compile.
using WiFiStateProducer = NetworkStateProducer;

}  // namespace sensesp

#endif  // SENSESP_NET_NETWORKING_H_
