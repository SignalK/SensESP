#include <memory>

#include "sensesp/system/max98357a.h"
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

// This is a sample program demonstrating the MAX98357A I2S amplifier driver.
// It plays a short two-tone chime every five seconds.

std::shared_ptr<MAX98357A> amplifier;

void setup() {
  SetupLogging();

  SensESPMinimalAppBuilder builder;
  auto sensesp_app = builder.set_hostname("max98357a-test")->get_app();

  amplifier = std::make_shared<MAX98357A>(
      MAX98357A_BCLK_PIN, MAX98357A_LRC_PIN, MAX98357A_DIN_PIN);
  if (!amplifier->begin()) {
    ESP_LOGE(__FILENAME__, "Failed to start the I2S output");
    return;
  }

  event_loop()->onRepeat(5000, []() {
    ESP_LOGI(__FILENAME__, "Playing chime");
    amplifier->play_tone(880, 300, 0.5);
    amplifier->play_tone(1320, 500, 0.5);
  });
}

void loop() { event_loop()->tick(); }
