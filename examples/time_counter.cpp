/**
 * @file constant_sensor.cpp
 * @brief Example of a TimeCounter that measures the time its input value is
 *   true on non-zero.
 *
 * The main use case for this transform is to measure the total engine hours.
 * The value is stored in the flash drive whenever the input state changes to
 * false (the engine is turned off).
 *
 * This example counts the input frequency of GPIO pin 15 and counts the
 * time it is non-zero.
 *
 * GPIO 18 is configured as output. The output frequency is increased every
 * 10 seconds. Connect pin 18 to pin 15 to test the example.
 *
 */

#include "sensesp/transforms/time_counter.h"

#include "sensesp/sensors/digital_input.h"
#include "sensesp/transforms/frequency.h"
#include "sensesp/transforms/lambda_transform.h"
#include "sensesp_app.h"
#include "sensesp_app_builder.h"

using namespace sensesp;

unsigned long cycle_start_time = 0;
unsigned long freq_start_time = 0;
int freq = 0;

void setup() {
  SetupLogging();

  // Create the builder object
  SensESPAppBuilder builder;
  sensesp_app = builder.get_app();

  // set GPIO 18 to output mode
  pinMode(18, OUTPUT);
  event_loop()->onRepeat(10, []() {
    if (freq == 0) {
      if (millis() - freq_start_time >= 10000) {
        freq = 10;
        freq_start_time = millis();
      } else {
        return;
      }
    } else {
      if (millis() - freq_start_time >= 1000) {
        freq += 10;
        freq_start_time = millis();
      }
      if (freq > 100) {
        freq = 0;
        return;
      }

      if ((millis() - cycle_start_time) >= 1000 / freq) {
        cycle_start_time = millis();
      } else {
        return;
      }
    }
    digitalWrite(18, !digitalRead(18));
  });

  // Create a digital input counter sensor
  auto digin_counter =
      new DigitalInputCounter(15, INPUT, FALLING, 500, "/Sensors/Counter");

  ConfigItem(digin_counter)->set_title("Input Counter")->set_sort_order(1000);

  // Create a frequency transform
  auto* frequency = new Frequency(1, "/Transforms/Frequency");
  digin_counter->connect_to(frequency);

  ConfigItem(frequency)->set_title("Frequency")->set_sort_order(1100);

  // create a propulsion state lambda transform
  auto* propulsion_state = new LambdaTransform<float, String>(
      [](bool freq) {
        if (freq > 0) {
          return "started";
        } else {
          return "stopped";
        }
      },
      "/Transforms/Propulsion State");

  ConfigItem(propulsion_state)
      ->set_title("Propulsion State")
      ->set_sort_order(1200);

  frequency->connect_to(propulsion_state);

  // create engine hours counter using PersistentDuration
  auto* engine_hours = new TimeCounter<float>("/Transforms/Engine Hours");

  ConfigItem(engine_hours)->set_title("Engine Hours")->set_sort_order(1300);

  ConfigItem(engine_hours)->set_title("Engine Hours")->set_sort_order(1300);

  frequency->connect_to(engine_hours);

  // create and connect the frequency output object
  frequency->connect_to(
      new SKOutput<float>("propulsion.main.revolutions", "",
                          new SKMetadata("Hz", "Main Engine Revolutions")));

  // create and connect the propulsion state output object
  propulsion_state->connect_to(new SKOutput<String>(
      "propulsion.main.state", "", new SKMetadata("", "Main Engine State")));

  // create and connect the engine hours output object
  engine_hours->connect_to(
      new SKOutput<float>("propulsion.main.runTime", "",
                          new SKMetadata("s", "Main Engine running time")));
}

void loop() { event_loop()->tick(); }
