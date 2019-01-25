#include "ota.h"

#include "../config.h"
#include "../sensesp.h"

#include <ArduinoOTA.h>
#include <Arduino.h>


void handle_OTA() {
  ArduinoOTA.handle();
}

void setup_OTA() {
#ifdef OTA_PASSWORD
  ArduinoOTA.setPassword((const char *)OTA_PASSWORD);
#endif
  ArduinoOTA.onStart([]() {
    Serial.println(F("Starting OTA"));
  });
  ArduinoOTA.onEnd([]() {
    Serial.println(F("\nEnd"));
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("OTA Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("OTA Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println(F("OTA Auth Failed"));
    else if (error == OTA_BEGIN_ERROR) Serial.println(F("OTA Begin Failed"));
    else if (error == OTA_CONNECT_ERROR) Serial.println(F("OTA Connect Failed"));
    else if (error == OTA_RECEIVE_ERROR) Serial.println(F("OTA Receive Failed"));
    else if (error == OTA_END_ERROR) Serial.println(F("OTA End Failed"));
  });
  ArduinoOTA.begin();
  //app.onTick(&handle_OTA);
  app.onRepeat(20, handle_OTA);
}
