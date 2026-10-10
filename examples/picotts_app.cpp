#include <memory>

#include "sensesp/system/max98357a.h"
#include "sensesp/system/pico_tts.h"
#include "sensesp_app.h"
#include "sensesp_app_builder.h"

using namespace sensesp;

// Pin defaults match the ESP32-S3 board configuration in platformio.ini.
#ifndef MAX98357A_LRC_PIN
#define MAX98357A_LRC_PIN 4
#endif
#ifndef MAX98357A_BCLK_PIN
#define MAX98357A_BCLK_PIN 5
#endif
#ifndef MAX98357A_DIN_PIN
#define MAX98357A_DIN_PIN 6
#endif

// This is a full SensESP application (WiFi, web UI, Signal K connection, OTA)
// with spoken output: PicoTTS text-to-speech through a MAX98357A I2S
// amplifier. It announces a shallow water warning in English and Italian once
// at boot; call `tts->say()` from your own sensors and transforms to speak.
//
// The voices are selected with `custom_picotts_languages = en-US, it-IT` in
// platformio.ini. Building requires the picotts library next to SensESP:
//   pio ci -e pioarduino_esp32s3 --project-conf platformio.ini \
//       --lib . --lib lib/picotts examples/picotts_app.cpp

std::shared_ptr<MAX98357A> amplifier;
std::shared_ptr<PicoTTS> tts;

void setup() {
  SetupLogging();

  // Create the global SensESPApp() object.
  SensESPAppBuilder builder;
  sensesp_app = builder.set_hostname("outlook-guardian")->get_app();

  amplifier = std::make_shared<MAX98357A>(
      MAX98357A_BCLK_PIN, MAX98357A_LRC_PIN, MAX98357A_DIN_PIN);
  if (!amplifier->begin()) {
    ESP_LOGE(__FILENAME__, "Failed to start the I2S output");
    return;
  }

  tts = std::make_shared<PicoTTS>(amplifier, "en-US");
  if (!tts->begin()) {
    return;
  }

  event_loop()->onDelay(2000, []() {
    tts->say("Caution shallow water", "en-US");
    tts->say("Attenzione acque basse", "it-IT");
  });
}

void loop() { event_loop()->tick(); }
