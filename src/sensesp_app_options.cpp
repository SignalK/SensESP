#include "sensesp_app_options.h"

SensESPAppOptions::SensESPAppOptions() { useDefault(); }

SensESPAppOptions* SensESPAppOptions::useDefault() {
  ssid = "";
  password = "";
  serverAddress = "";
  serverPort = 0;
  enabledSensors = all;
  mDNSEnabled = true;

#ifdef LED_BUILTIN
  ledPin = LED_BUILTIN;
  enableLED = true;
#else
  enableLED = false;
  ledPin = 0;
  debugW("LED_BUILTIN isn't defined! LED blink will be disabled!");
#endif

  hostName = "sensesp";

  return this;
}

SensESPAppOptions* SensESPAppOptions::setWifiOptions(String ssid,
                                                     String password) {
  this->ssid = ssid;
  this->password = password;

  return this;
}

SensESPAppOptions* SensESPAppOptions::setServerOptions(String address,
                                                       int port) {
  serverAddress = address;
  serverPort = port;
  mDNSEnabled = false;

  return this;
}

SensESPAppOptions* SensESPAppOptions::setStandardSensors(
    StandardSensorsOptions_t sensors) {
  enabledSensors = sensors;
  return this;
}

SensESPAppOptions* SensESPAppOptions::setLEDOptions(bool enabled,
                                                    int webSocketConnected,
                                                    int wifiConnected,
                                                    int offline) {
  enableLED = enabled;
  ledIntervals.wifiConnected = wifiConnected;
  ledIntervals.websocketConnected = webSocketConnected;
  ledIntervals.offlineInterval = offline;

  return this;
}

SensESPAppOptions* SensESPAppOptions::setHostName(String hostName) {
  this->hostName = hostName;
  this->isHostNameChanged = true;
  return this;
}

SensESPAppOptions* SensESPAppOptions::useMDNS() {
  this->serverAddress = "";
  this->serverPort = 0;
  return this;
}
