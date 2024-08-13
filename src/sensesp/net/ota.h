#ifndef _ota_H_
#define _ota_H_

#include "sensesp.h"

#include <Arduino.h>
#include <ArduinoOTA.h>

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
      ArduinoOTA.onStart([]() { ESP_LOGW(__FILENAME__, "Starting OTA"); });
      ArduinoOTA.onEnd([]() { ESP_LOGW(__FILENAME__, "OTA End"); });
      ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        ESP_LOGI(__FILENAME__, "OTA Progress: %u%%\r", (progress / (total / 100)));
      });
      ArduinoOTA.onError([](ota_error_t error) {
        ESP_LOGE(__FILENAME__, "OTA Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) {
          ESP_LOGE(__FILENAME__, "OTA Auth Failed");
        } else if (error == OTA_BEGIN_ERROR) {
          ESP_LOGE(__FILENAME__, "OTA Begin Failed");
        } else if (error == OTA_CONNECT_ERROR) {
          ESP_LOGE(__FILENAME__, "OTA Connect Failed");
        } else if (error == OTA_RECEIVE_ERROR) {
          ESP_LOGE(__FILENAME__, "OTA Receive Failed");
        } else if (error == OTA_END_ERROR) {
          ESP_LOGE(__FILENAME__, "OTA End Failed");
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
