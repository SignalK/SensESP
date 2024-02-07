#ifndef _ota_H_
#define _ota_H_

#include <Arduino.h>
#include <ArduinoOTA.h>

#include "sensesp.h"

namespace sensesp {

class OTA {
 public:
  /**
   * @brief Construct a new OTA (Over-the-air update) object
   *
   * @param password A password to be used for the OTA update.
   */
  OTA(const char* password) : password_{password} {
    ReactESP::app->onDelay(0, [this]() {
      ArduinoOTA.setPassword(password_);
      ArduinoOTA.onStart([]() { debugW("Starting OTA"); });
      ArduinoOTA.onEnd([]() { debugW("OTA End"); });
      ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        debugI("OTA Progress: %u%%\r", (progress / (total / 100)));
      });
      ArduinoOTA.onError([](ota_error_t error) {
        debugE("OTA Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) {
          debugE("OTA Auth Failed");
        } else if (error == OTA_BEGIN_ERROR) {
          debugE("OTA Begin Failed");
        } else if (error == OTA_CONNECT_ERROR) {
          debugE("OTA Connect Failed");
        } else if (error == OTA_RECEIVE_ERROR) {
          debugE("OTA Receive Failed");
        } else if (error == OTA_END_ERROR) {
          debugE("OTA End Failed");
        }
      });
      ArduinoOTA.begin();
      ReactESP::app->onRepeat(20, OTA::handle_ota);
    });
  }

 private:
  const char* password_;
  static void handle_ota();
};

}  // namespace sensesp

#endif
