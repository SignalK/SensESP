#include <Arduino.h>

#include "sensesp_app.h"
#include "sensesp_app_builder.h"
#include "sensors/digital_input.h"
#include "signalk/signalk_output.h"
#include "system/lambda_consumer.h"
#include "transforms/debounce.h"
#include "transforms/integrator.h"

using namespace sensesp;

/**
 * This example illustrates an anchor chain counter. Note that it
 * doesn't distinguish between chain being let out and chain being
 * taken in, so the intended usage is this: Press the button to make
 * sure the counter is at 0.0. Let out chain until the counter shows
 * the amount you want out. Once the anchor is set, press the button
 * again to reset the counter to 0.0. As you bring the chain in, the
 * counter will show how much you have brought in. Press the button
 * again to reset the counter to 0.0, to be ready for the next anchor
 * deployment.
 *
 * A bi-directional chain counter is possible, but this is not one.
 */

ReactESP app;

void setup() {
  SetupSerialDebug(115200);

  SensESPAppBuilder builder;
  sensesp_app = builder.set_hostname("ChainCounter")
                    ->set_wifi("YourSSID", "YourPassword")
                    ->get_app();

#ifdef ESP8266
  uint8_t COUNTER_PIN = D7;
  uint8_t BUTTON_PIN = D5;
#elif defined(ESP32)
  uint8_t COUNTER_PIN = 32;
  uint8_t BUTTON_PIN = 34;
#endif

  /**
   * DigitalInputCounter will count the revolutions of the windlass with a
   * Hall Effect Sensor connected to COUNTER_PIN. It will output its count
   * every counter_read_delay ms, which can be configured in the Config UI at
   * counter_config_path.
   */
  unsigned int counter_read_delay = 1000;
  String counter_config_path = "/chain_counter/read_delay";
  auto* chain_counter =
      new DigitalInputCounter(COUNTER_PIN, INPUT_PULLUP, RISING,
                              counter_read_delay, counter_config_path);

  /**
   * An IntegratorT<int, float> called "accumulator" adds up all the counts it
   * receives (which are ints) and multiplies each count by gypsy_circum, which
   * is the amount of chain, in meters, that is moved by each revolution of the
   * windlass. (Since gypsy_circum is a float, the output of this transform must
   * be a float, which is why we use IntegratorT<int, float>). It can be
   * configured in the Config UI at accum_config_path.
   */
  float gypsy_circum = 0.32;
  String accum_config_path = "/accumulator/circum";
  auto* accumulator =
      new IntegratorT<int, float>(gypsy_circum, 0.0, accum_config_path);

  /**
   * There is no path for the amount of anchor rode deployed in the current
   * Signal K specification. By creating an instance of SKMetaData, we can send
   * a partial or full defintion of the metadata that other consumers of Signal
   * K data might find useful. (For example, Instrument Panel will benefit from
   * knowing the units to be displayed.) The metadata is sent only the first
   * time the data value is sent to the server.
   */
  SKMetadata* metadata = new SKMetadata();
  metadata->units_ = "m";
  metadata->description_ = "Anchor Rode Deployed";
  metadata->display_name_ = "Rode Deployed";
  metadata->short_name_ = "Rode Out";

  /**
   * chain_counter is connected to accumulator, which is connected to an
   * SKOutputNumber, which sends the final result to the indicated path on the
   * Signal K server. (Note that each data type has its own version of SKOutput:
   * SKOutputNumber for floats, SKOutputInt, SKOutputBool, and SKOutputString.)
   */
  String sk_path = "navigation.anchor.rodeDeployed";
  String sk_path_config_path = "/rodeDeployed/sk";

  chain_counter->connect_to(accumulator)
      ->connect_to(new SKOutputFloat(sk_path, sk_path_config_path, metadata));

  /**
   * DigitalInputChange monitors a physical button connected to BUTTON_PIN.
   * Because its interrupt type is CHANGE, it will emit a value when the button
   * is pressed, and again when it's released, but that's OK - our
   * LambdaConsumer function will act only on the press, and ignore the release.
   * DigitalInputChange looks for a change every read_delay ms, which can be
   * configured at read_delay_config_path in the Config UI.
   */
  int read_delay = 10;
  String read_delay_config_path = "/button_watcher/read_delay";
  auto* button_watcher = new DigitalInputChange(
      BUTTON_PIN, INPUT, read_delay, read_delay_config_path);

  /**
   * Create a DebounceInt to make sure we get a nice, clean signal from the
   * button. Set the debounce delay period to 15 ms, which can be configured at
   * debounce_config_path in the Config UI.
   */
  int debounce_delay = 15;
  String debounce_config_path = "/debounce/delay";
  auto* debounce = new DebounceInt(debounce_delay, debounce_config_path);

  /**
   * When the button is pressed (or released), it will call the lambda
   * expression (or "function") that's called by the LambdaConsumer. This is the
   * function - notice that it calls reset() only when the input is 1, which
   * indicates a button press. It ignores the button release. If your button
   * goes to GND when pressed, make it "if (input == 0)".
   */
  auto reset_function = [accumulator](int input) {
    if (input == 1) {
      accumulator->reset();  // Resets the output to 0.0
    }
  };

  /**
   * Create the LambdaConsumer that calls reset_function, Because
   DigitalInputChange
   * outputs an int, the version of LambdaConsumer we need is
   LambdaConsumer<int>.
   *
   * While this approach - defining the lambda function (above) separate from
   the
   * LambdaConsumer (below) - is simpler to understand, there is a more concise
   approach:
   *
    auto* button_consumer = new LambdaConsumer<int>([accumulator](int input) {
      if (input == 1) {
        accumulator->reset();
      }
    });

   *
  */
  auto* button_consumer = new LambdaConsumer<int>(reset_function);

  /* Connect the button_watcher to the debounce to the button_consumer. */
  button_watcher->connect_to(debounce)->connect_to(button_consumer);

  /* Finally, start the SensESPApp */
  sensesp_app->start();
}

// The loop function is called in an endless loop during program execution.
// It simply calls `app.tick()` which will then execute all reactions as needed.
void loop() { app.tick(); }
