#ifndef SENSESP_NET_WIFI_PROVISIONER_H_
#define SENSESP_NET_WIFI_PROVISIONER_H_

#include <Arduino.h>
#include <DNSServer.h>
#include <WiFi.h>

#include <memory>
#include <vector>

#include "sensesp/net/network_provisioner.h"
#include "sensesp/net/network_state.h"
#include "sensesp/system/observablevalue.h"
#include "sensesp/system/resettable.h"
#include "sensesp/system/serializable.h"
#include "sensesp_base_app.h"

namespace sensesp {

constexpr int kMaxNumClientConfigs = 3;

/**
 * @brief Storage object for WiFi access point settings.
 */
class AccessPointSettings {
 public:
  AccessPointSettings(bool enabled = true, String ssid = "",
                      String password = "", int channel = 9,
                      bool hidden = false, bool captive_portal_enabled = true)
      : enabled_{enabled},
        ssid_{ssid},
        password_{password},
        channel_{channel},
        hidden_{hidden},
        captive_portal_enabled_{captive_portal_enabled} {
    if (ssid_ == "") {
      // If no SSID is provided, use the hostname as the SSID
      ssid_ = SensESPBaseApp::get_hostname();
    }
  };

  bool enabled_;
  String ssid_;
  String password_;
  int channel_;
  bool hidden_;
  bool captive_portal_enabled_ = true;

  static AccessPointSettings from_json(const JsonObject& json) {
    AccessPointSettings settings;
    settings.enabled_ = json["enabled"] | false;
    settings.ssid_ = json["name"] | "";
    settings.password_ = json["password"] | "";
    settings.channel_ = json["channel"] | 1;
    settings.hidden_ = json["hidden"] | false;
    settings.captive_portal_enabled_ = json["captivePortalEnabled"] | true;
    return settings;
  }

  void as_json(JsonObject& doc) {
    doc["enabled"] = enabled_;
    doc["name"] = ssid_;
    doc["password"] = password_;
    doc["channel"] = channel_;
    doc["hidden"] = hidden_;
    doc["captivePortalEnabled"] = captive_portal_enabled_;
  }
};

/**
 * @brief Storage object for WiFi client settings.
 */
class ClientSSIDConfig {
 public:
  ClientSSIDConfig(String ssid = "", String password = "", bool use_dhcp = true,
                   IPAddress ip = IPAddress(169, 254, 0, 1),
                   IPAddress netmask = IPAddress(255, 255, 255, 0),
                   IPAddress gateway = IPAddress(192, 168, 0, 1),
                   IPAddress dns_server = IPAddress(8, 8, 8, 8))
      : ssid_{ssid},
        password_{password},
        use_dhcp_{use_dhcp},
        ip_{ip},
        netmask_{netmask},
        gateway_{gateway},
        dns_server_{dns_server} {};

  String ssid_;
  String password_;
  bool use_dhcp_;
  IPAddress ip_;
  IPAddress netmask_;
  IPAddress gateway_;
  IPAddress dns_server_;

  static ClientSSIDConfig from_json(const JsonObject& json) {
    ClientSSIDConfig config;
    config.ssid_ = json["name"] | "";
    config.password_ = json["password"] | "";
    config.use_dhcp_ = json["useDHCP"] | true;
    config.ip_.fromString(json["ipAddress"] | "169.254.0.1");
    config.netmask_.fromString(json["netmask"] | "255.255.255.0");
    config.gateway_.fromString(json["gateway"] | "192.168.0.1");
    config.dns_server_.fromString(json["dnsServer"] | "8.8.8.8");
    return config;
  }

  void as_json(JsonObject& doc) {
    doc["name"] = ssid_;
    doc["password"] = password_;
    doc["useDHCP"] = use_dhcp_;
    doc["ipAddress"] = ip_.toString();
    doc["netmask"] = netmask_.toString();
    doc["gateway"] = gateway_.toString();
    doc["dnsServer"] = dns_server_.toString();
  }
};

/**
 * @brief WiFi network info storage class returned by scan results.
 */
class WiFiNetworkInfo {
 public:
  WiFiNetworkInfo()
      : ssid_{""}, rssi_{0}, encryption_{0}, bssid_{0}, channel_{0} {}
  WiFiNetworkInfo(String ssid, int32_t rssi, uint8_t encryption, uint8_t* bssid,
                  int32_t channel)
      : ssid_{ssid}, rssi_{rssi}, encryption_{encryption}, channel_{channel} {
    if (bssid != nullptr) {
      memcpy(bssid_, bssid, 6);
    } else {
      memset(bssid_, 0, 6);
    }
  }

  String ssid_;
  int32_t rssi_;
  uint8_t encryption_;
  uint8_t bssid_[6];
  int32_t channel_;

  void as_json(JsonObject& doc) {
    doc["ssid"] = ssid_;
    doc["rssi"] = rssi_;
    doc["encryption"] = encryption_;
    doc["channel"] = channel_;
    // Zero-pad each byte so MAC addresses like 02:0A:... format
    // correctly. The pre-refactor code used `String(byte, HEX)` which
    // produces "A" instead of "0A" for single-digit hex values.
    char bssid_str[18];
    snprintf(bssid_str, sizeof(bssid_str),
             "%02X:%02X:%02X:%02X:%02X:%02X", bssid_[0], bssid_[1],
             bssid_[2], bssid_[3], bssid_[4], bssid_[5]);
    doc["bssid"] = bssid_str;
  }
};

/**
 * @brief Manages the ESP's WiFi connection.
 *
 * Inherits from NetworkProvisioner so SensESPApp can hold any provisioner
 * implementation through the same shared_ptr type. Adds WiFi-specific
 * methods (scanning, soft-AP IP, captive portal flag) directly on the
 * concrete class — code that needs them holds a typed pointer via
 * SensESPApp::get_wifi_provisioner().
 *
 * The legacy `Networking` class name is preserved as a typedef in
 * networking.h for source-level backward compatibility.
 */
class WiFiProvisioner : public FileSystemSaveable,
                       public Resettable,
                       public NetworkProvisioner,
                       public ValueProducer<WiFiState> {
 public:
  WiFiProvisioner(const String& config_path, const String& client_ssid = "",
                  const String& client_password = "",
                  const String& ap_ssid = "",
                  const String& ap_password = "");
  ~WiFiProvisioner() override;

  // -- Resettable --
  void reset() override;

  // -- FileSystemSaveable --
  bool to_json(JsonObject& doc) override;
  bool from_json(const JsonObject& config) override;

  // -- NetworkProvisioner --
  IPAddress local_ip() const override { return WiFi.localIP(); }
  IPAddress gateway_ip() const override { return WiFi.gatewayIP(); }
  String mac_address() const override { return WiFi.macAddress(); }
  bool is_connected() const override {
    return WiFi.isConnected() || WiFi.getMode() == WIFI_MODE_AP ||
           WiFi.getMode() == WIFI_MODE_APSTA;
  }

  /**
   * @deprecated Use SensESPApp::get_network_state_producer() or
   * connect_to() on this WiFiProvisioner directly (it is now a
   * ValueProducer<WiFiState>). Kept for source compatibility with
   * code that called get_networking()->get_wifi_state_producer().
   */
  ValueProducer<WiFiState>* get_wifi_state_producer() { return this; }

  // -- WiFi-specific methods --
  IPAddress soft_ap_ip() const { return WiFi.softAPIP(); }
  bool is_captive_portal_enabled() const {
    return ap_settings_.captive_portal_enabled_;
  }
  String ssid() const { return WiFi.SSID(); }
  int rssi() const { return WiFi.RSSI(); }

  void start_wifi_scan();
  int16_t get_wifi_scan_results(std::vector<WiFiNetworkInfo>& ssid_list);

 protected:
  void start_access_point();
  void start_client_autoconnect();

  AccessPointSettings ap_settings_;

  bool client_enabled_ = false;
  std::vector<ClientSSIDConfig> client_settings_;

  std::unique_ptr<DNSServer> dns_server_;
};

inline const String ConfigSchema(const WiFiProvisioner& obj) { return "null"; }

inline bool ConfigRequiresRestart(const WiFiProvisioner& obj) { return true; }

}  // namespace sensesp

#endif  // SENSESP_NET_WIFI_PROVISIONER_H_
