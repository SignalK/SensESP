#include <memory>

#include "sensesp/system/max98357a.h"
#include "sensesp/system/pico_tts.h"
#include "sensesp_minimal_app_builder.h"

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

// This is a sample program demonstrating bilingual PicoTTS text-to-speech
// output through a MAX98357A I2S amplifier. It announces a shallow water
// warning in English and then in Italian, at boot and every 15 seconds.
//
// The voices are selected with `custom_picotts_languages = en-US, it-IT` in
// platformio.ini. Building requires the picotts library next to SensESP:
//   pio ci -e pioarduino_esp32s3 --project-conf platformio.ini \
//       --lib . --lib lib/picotts examples/picotts_shallow_water.cpp

std::shared_ptr<MAX98357A> amplifier;
std::shared_ptr<PicoTTS> tts;

void announce() {
  ESP_LOGI(__FILENAME__, "Announcing shallow water");
  tts->say("Caution shallow water", "en-US");
  tts->say("Attenzione acque basse", "it-IT");
}

void setup() {
  SetupLogging();

  SensESPMinimalAppBuilder builder;
  auto sensesp_app = builder.set_hostname("picotts-test")->get_app();

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

  event_loop()->onDelay(1000, announce);
  event_loop()->onRepeat(15000, announce);
}

void loop() { event_loop()->tick(); }
