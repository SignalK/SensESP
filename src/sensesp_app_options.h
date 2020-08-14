#ifndef _app_options_H_
#define _app_options_H_
#include "sensesp.h"

enum StandardSensorsOptions_t {
  none,
  uptime = 0x01,
  frequency = 0x02,
  freeMemory = 0x04,
  ipAddress = 0x08,
  wifiSignal = 0x10,
  all = 0x1F
};

struct LedIntervals_T {
  int websocketConnected;
  int wifiConnected;
  int offlineInterval;
};

class SensESPAppOptions {
 public:
  SensESPAppOptions();
  SensESPAppOptions* useDefault();
  SensESPAppOptions* setWifiOptions(String ssid, String password);
  SensESPAppOptions* setServerOptions(String serverAddress, int port);
  SensESPAppOptions* setStandardSensors(StandardSensorsOptions_t sensors = all);
  SensESPAppOptions* useMDNS();
  SensESPAppOptions* setLEDOptions(bool enabled = true,
                                   int webSocketConnected = 200,
                                   int wifiConnected = 1000,
                                   int offline = 5000);
  SensESPAppOptions* setLEDPin(int pin);
  SensESPAppOptions* setHostName(String hostname);

  String getSsid() { return ssid; }
  String getPassword() { return password; }
  String getServerAddress() { return serverAddress; }
  int getServerPort() { return serverPort; }
  bool getMDNSEnabled() { return mDNSEnabled; }
  StandardSensorsOptions_t getStandardSensors() { return enabledSensors; }
  bool getLEDEnabled() { return enableLED; }
  int getLEDPin() { return ledPin; }
  bool isWifiSet() { return !ssid.isEmpty(); }
  bool isServerSet() { return !serverAddress.isEmpty(); }
  LedIntervals_T getLEDIntervals() { return ledIntervals; }
  String getHostname() { return hostName; }
  bool isHostNameSet() { return isHostNameChanged; }

 private:
  String ssid;
  String password;
  String serverAddress;
  bool isHostNameChanged;
  String hostName;
  int serverPort;
  StandardSensorsOptions_t enabledSensors;
  bool mDNSEnabled;
  bool enableLED;
  LedIntervals_T ledIntervals;
  int ledPin;
};

#endif