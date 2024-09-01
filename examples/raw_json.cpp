/**
 * @brief Example and test program for using Raw Json output.
 *
 */

#include "sensesp/system/lambda_consumer.h"
#include "sensesp/transforms/lambda_transform.h"
#include "sensesp_app_builder.h"

using namespace sensesp;

ObservableValue<bool> toggler;

void setup() {
  SetupLogging();

  SensESPAppBuilder builder;
  SensESPApp *sensesp_app = builder.set_hostname("json_demo")
                                ->set_wifi("Hat Labs Sensors", "kanneluuri2406")
                                ->get_app();

  SensESPBaseApp::get_event_loop()->onRepeat(1000,
                                    []() { toggler.set(!toggler.get()); });

  // take some boolean input and convert it into a simple serialized JSON
  // document
  auto jsonify = new LambdaTransform<bool, String>([](bool input) -> String {
    JsonDocument doc;
    doc["output_1"] = input;
    String output;
    serializeJson(doc, output);
    return output;
  });

  toggler.connect_to(jsonify);

  // print the JSON document to the serial console
  jsonify->connect_to(new LambdaConsumer<String>([](String input) {
    ESP_LOGD("Example", "JSONified output: %s", input.c_str());
  }));

  // connect jsonify to the SK delta queue

  const char *sk_path = "environment.json.pin15";
  jsonify->connect_to(new SKOutputRawJson(sk_path, ""));
}

void loop() {
  // We're storing the event loop in a static variable so that it's only
  // acquired once. Saves a few function calls per loop iteration.
  static auto event_loop = SensESPBaseApp::get_event_loop();
  event_loop->tick();
}
