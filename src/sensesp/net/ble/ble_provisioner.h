#ifndef SENSESP_NET_BLE_BLE_PROVISIONER_H_
#define SENSESP_NET_BLE_BLE_PROVISIONER_H_

#include <Arduino.h>

#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

#include "sensesp/net/ble/ble_advertisement.h"
#include "sensesp/system/valueproducer.h"

namespace sensesp {

/// Callbacks delivered by the BLE provisioner for an active GATT
/// connection. All callbacks fire from the BT stack task — they must
/// not block. The provisioner owns the callback lifetime for as long
/// as the connection is open.
struct GATTConnectionCallbacks {
  /// Connection established and service/characteristic discovery done.
  std::function<void()> on_connected;
  /// Connection dropped (peer-initiated or link loss).
  std::function<void(const String& reason)> on_disconnected;
  /// Notification received from a characteristic.
  std::function<void(const String& char_uuid,
                     const uint8_t* data, size_t len)> on_notify;
  /// Read response received.
  std::function<void(const String& char_uuid,
                     const uint8_t* data, size_t len)> on_read;
  /// Write completed (success or failure).
  std::function<void(const String& char_uuid, bool success)> on_write_complete;
  /// Unrecoverable error during connection setup.
  std::function<void(const String& error)> on_error;
};

/**
 * @brief Chip-agnostic BLE provisioner interface.
 *
 * A BLEProvisioner is responsible for bringing the Bluetooth host
 * stack and its underlying transport up, running a BLE central scan,
 * and (eventually) acting as a GATT client for GATT-enabled consumers
 * such as BLESignalKGateway.
 *
 * This interface deliberately does not expose stack-specific types
 * (no NimBLE, no Bluedroid, no esp_hosted types in the signature).
 * The scanner emits plain BLEAdvertisement values through the
 * inherited ValueProducer<BLEAdvertisement>. Consumers attach via the
 * usual SensESP connect_to() / attach() plumbing.
 *
 * Concrete implementations live alongside this header in
 * sensesp/net/ble/. Each is guarded by its own #if
 * defined(CONFIG_...) block against the kconfig symbols that signal
 * its prerequisites are satisfied — for example, EspHostedBluedroidBLE
 * is only declared when both Bluedroid and esp_hosted's Bluedroid
 * VHCI transport are enabled in the build's sdkconfig.
 *
 * ## Lifecycle
 *
 * The BLE provisioner's constructor is expected to do the full bring-
 * up synchronously: initialise the controller transport (e.g.
 * hostedInitBLE() on esp_hosted), initialise the host stack, and
 * register callbacks. Scanning is not started by the constructor —
 * callers invoke start_scan() explicitly once they are ready to
 * receive advertisements.
 *
 * This matches the pattern the rest of SensESP uses (see
 * NetworkProvisioner / EthernetProvisioner) where the construction
 * itself is the "bring-up" step and there is no separate begin()
 * method.
 */
class BLEProvisioner : public ValueProducer<BLEAdvertisement> {
 public:
  virtual ~BLEProvisioner() = default;

  /**
   * @brief Start a BLE central scan.
   *
   * Idempotent — calling while already scanning is a no-op that
   * returns true. Each received advertisement is emitted through the
   * inherited ValueProducer<BLEAdvertisement>, so observers that
   * connect_to() this provisioner will see every advertisement as
   * the host stack's GAP callback runs.
   *
   * @return true if the scan is running (or was already running)
   *         after this call, false if the request failed.
   */
  virtual bool start_scan() = 0;

  /**
   * @brief Stop an in-progress BLE central scan.
   *
   * Idempotent — calling while not scanning is a no-op that returns
   * true. After this call, no more BLEAdvertisement values are
   * emitted until start_scan() is called again.
   */
  virtual bool stop_scan() = 0;

  /// True iff the scanner is currently running.
  virtual bool is_scanning() const = 0;

  /// Hardware MAC address of the local BT controller, formatted
  /// "AA:BB:CC:DD:EE:FF". Returns an empty string if the controller
  /// has not yet reported its address.
  virtual String mac_address() const = 0;

  /**
   * @brief Number of advertisements received since boot.
   *
   * Useful for debugging silent scans: if this is zero after several
   * seconds of scanning in an environment with known BLE activity,
   * something is wrong with either the host stack, the controller,
   * or the transport between them — not with scan parameters.
   */
  virtual uint32_t scan_hit_count() const = 0;

  /**
   * @brief Reset the BT controller and reinitialise the host stack.
   *
   * This is a heavy recovery operation for when the remote BT
   * controller (e.g. ESP32-C6 over esp_hosted SDIO) has stalled and
   * a simple scan stop/start doesn't recover it. The default
   * implementation is a no-op; concrete provisioners that support
   * controller reset should override this.
   *
   * @return true if the controller was successfully reset and the
   *         stack is ready for start_scan() again.
   */
  virtual bool reset_bt_controller() { return false; }

  /**
   * @brief Hard-reset the remote co-processor via GPIO.
   *
   * Even more aggressive than reset_bt_controller() — this toggles the
   * companion chip's hardware reset line, power-cycling it entirely.
   * Use this when RPC-based resets don't recover the scan stall. The
   * default implementation is a no-op; only provisioners that talk to
   * a remote co-processor (e.g. EspHostedBluedroidBLE) implement this.
   *
   * @return true if the co-processor was successfully reset and the
   *         BLE stack is ready for start_scan() again.
   */
  virtual bool hard_reset_c6() { return false; }

  // -----------------------------------------------------------------
  // GATT client interface
  // -----------------------------------------------------------------

  /// Maximum concurrent GATT connections this provisioner supports.
  virtual int max_gatt_connections() const { return 0; }

  /// Number of currently active GATT connections.
  virtual int active_gatt_connections() const { return 0; }

  /**
   * @brief Connect to a BLE peripheral and discover a service.
   *
   * Initiates a GATT connection to the given MAC address. After
   * connecting, the provisioner discovers the specified service and
   * enumerates its characteristics. When ready, callbacks.on_connected
   * fires. Notification/read/write operations can then be requested.
   *
   * @param mac       "AA:BB:CC:DD:EE:FF" format
   * @param addr_type BLE address type (0=public, 1=random)
   * @param service_uuid UUID of the primary service to discover
   * @param callbacks Event callbacks for this connection
   * @return connection handle (>= 0) on success, -1 on failure
   */
  virtual int gatt_connect(const String& mac, uint8_t addr_type,
                           const String& service_uuid,
                           GATTConnectionCallbacks callbacks) {
    return -1;
  }

  /// Subscribe to notifications on a characteristic.
  virtual bool gatt_subscribe_notify(int conn_handle,
                                     const String& char_uuid) {
    return false;
  }

  /// Read a characteristic value. Result arrives via on_read callback.
  virtual bool gatt_read(int conn_handle, const String& char_uuid) {
    return false;
  }

  /// Write to a characteristic. Result via on_write_complete callback.
  virtual bool gatt_write(int conn_handle, const String& char_uuid,
                          const uint8_t* data, size_t len) {
    return false;
  }

  /// Disconnect a GATT connection and release resources.
  virtual void gatt_disconnect(int conn_handle) {}
};

}  // namespace sensesp

#endif  // SENSESP_NET_BLE_BLE_PROVISIONER_H_
