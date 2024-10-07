#ifndef SENSESP_NET_NETWORKING_H_
#define SENSESP_NET_NETWORKING_H_

#include <Arduino.h>
#include <DNSServer.h>
#include <WiFi.h>

#include "sensesp/net/wifi_state.h"
#include "sensesp/system/serializable.h"
#include "sensesp/system/observablevalue.h"
#include "sensesp/system/resettable.h"
#include "sensesp/system/valueproducer.h"
#include "sensesp_base_app.h"

namespace sensesp {

constexpr int kMaxNumClientConfigs = 3;

/**
 * @brief Provide information about the current WiFi state.
 *
 * WiFiStateProducer reads the current network state using
 * Arduino Core callbacks. It is a replacement for the Networking class
 * ValueProducer output and effectively decouples the Networkig class
 * from the rest of the system. This allows for replacing the Networking
 * class with a different implementation.
 */
class WiFiStateProducer : public ValueProducer<WiFiState> {
 public:
  /**
   * Singletons should not be cloneable
   */
  WiFiStateProducer(WiFiStateProducer& other) = delete;

  /**
   * Singletons should not be assignable
   */
  void operator=(const WiFiStateProducer&) = delete;

  /**
   * @brief Get the singleton instance of the WiFiStateProducer
   */
  static WiFiStateProducer* get_singleton();

 protected:
  WiFiStateProducer() {
    this->output_ = WiFiState::kWifiNoAP;

    setup_wifi_callbacks();

    // Emit the current state as soon as the event loop starts
    event_loop()->onDelay(0, [this]() { this->emit(this->output_); });
  }

  void setup_wifi_callbacks() {
    WiFi.onEvent(
        [this](WiFiEvent_t event, WiFiEventInfo_t info) {
          this->wifi_station_connected();
        },
        WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
    WiFi.onEvent([this](WiFiEvent_t event,
                        WiFiEventInfo_t info) { this->wifi_ap_enabled(); },
                 WiFiEvent_t::ARDUINO_EVENT_WIFI_AP_START);
    WiFi.onEvent([this](WiFiEvent_t event,
                        WiFiEventInfo_t info) { this->wifi_disconnected(); },
                 WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
    WiFi.onEvent([this](WiFiEvent_t event,
                        WiFiEventInfo_t info) { this->wifi_disconnected(); },
                 WiFiEvent_t::ARDUINO_EVENT_WIFI_AP_STOP);
  }

  void wifi_station_connected() {
    ESP_LOGI(__FILENAME__, "Connected to wifi, SSID: %s (signal: %d)",
             WiFi.SSID().c_str(), WiFi.RSSI());
    ESP_LOGI(__FILENAME__, "IP address of Device: %s",
             WiFi.localIP().toString().c_str());
    ESP_LOGI(__FILENAME__, "Default route: %s",
             WiFi.gatewayIP().toString().c_str());
    ESP_LOGI(__FILENAME__, "DNS server: %s", WiFi.dnsIP().toString().c_str());
    this->emit(WiFiState::kWifiConnectedToAP);
  }

  void wifi_ap_enabled() {
    ESP_LOGI(__FILENAME__, "WiFi Access Point enabled, SSID: %s",
             WiFi.softAPSSID().c_str());
    ESP_LOGI(__FILENAME__, "IP address of Device: %s",
             WiFi.softAPIP().toString().c_str());

    // Setting the AP mode happens immediately,
    // so this callback is likely called already before all startables have been
    // initiated. Delay the WiFi state update until the start of the event loop.
    event_loop()->onDelay(
        0, [this]() { this->emit(WiFiState::kWifiAPModeActivated); });
  }

  void wifi_disconnected() {
    ESP_LOGI(__FILENAME__, "Disconnected from wifi.");
    this->emit(WiFiState::kWifiDisconnected);
  }

  static WiFiStateProducer* instance_;
};

/**
 * @brief Storage object for WiFi access point settings.
 *
 */
class AccessPointSettings {
 public:
  AccessPointSettings(bool enabled = true, String ssid = "",
                      String password = "thisisfine", int channel = 9,
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
 *
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
 * @brief WiFi Network Information storage class.
 *
 */
class WiFiNetworkInfo {
 public:
  WiFiNetworkInfo()
      : ssid_{""}, rssi_{0}, encryption_{0}, bssid_{0}, channel_{0} {}
  WiFiNetworkInfo(String ssid, int32_t rssi, uint8_t encryption, uint8_t* bssid,
                  int32_t channel)
      : ssid_{ssid}, rssi_{rssi}, encryption_{encryption}, channel_{channel} {
    memcpy(bssid_, bssid, 6);
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
    // Add bssid as a string
    doc["bssid"] = String(bssid_[0], HEX) + ":" + String(bssid_[1], HEX) + ":" +
                   String(bssid_[2], HEX) + ":" + String(bssid_[3], HEX) + ":" +
                   String(bssid_[4], HEX) + ":" + String(bssid_[5], HEX);
  }
};

/**
 * @brief Manages the ESP's connection to the Wifi network.
 */
class Networking : virtual public FileSystemSaveable,
                   public Resettable,
                   public ValueProducer<WiFiState> {
 public:
  Networking(String config_path, String client_ssid = "",
             String client_password = "");
  virtual void reset() override;

  virtual bool to_json(JsonObject& doc) override;
  virtual bool from_json(const JsonObject& config) override;

  void start_wifi_scan();
  int16_t get_wifi_scan_results(std::vector<WiFiNetworkInfo>& ssid_list);

  bool is_captive_portal_enabled() {
    return ap_settings_.captive_portal_enabled_;
  }

 protected:
  void start_access_point();
  void start_client_autoconnect();

  // callbacks

  void wifi_station_connected();
  void wifi_ap_enabled();
  void wifi_disconnected();

 private:
  AccessPointSettings ap_settings_;

  bool client_enabled_ = false;
  std::vector<ClientSSIDConfig> client_settings_;

  DNSServer* dns_server_ = nullptr;

  WiFiStateProducer* wifi_state_producer;
};

inline bool ConfigRequiresRestart(const Networking& obj) { return true; }

}  // namespace sensesp

#endif
