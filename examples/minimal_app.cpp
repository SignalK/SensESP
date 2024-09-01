#include <math.h>

#include "sensesp/net/http_server.h"
#include "sensesp/net/networking.h"
#include "sensesp/sensors/digital_input.h"
#include "sensesp/transforms/lambda_transform.h"
#include "sensesp/transforms/linear.h"
#include "sensesp/transforms/typecast.h"
#include "sensesp_minimal_app_builder.h"

using namespace sensesp;

const unsigned int read_delay = 500;

const uint8_t input_pin1 = 15;
const uint8_t output_pin1 = 18;
const uint8_t input_pin2 = 13;
const uint8_t output_pin2 = 21;

// This is a sample program to demonstrate how to instantiate a
// SensESPMinimalApp application and only enable some required components
// explicitly.
//
// The program just wiggles GPIO pins 18 and 21 at defined rates, and
// reads the input of pins 15 and 13. If the output and input pins are connected
// with a jumper wire, you should see changes in the reported values on the
// serial console.

reactesp::ReactESP app;

void setup() {
  SetupLogging();

  SensESPMinimalAppBuilder builder;
  auto sensesp_app = builder.set_hostname("counter-test")->get_app();

  // manually create Networking and HTTPServer objects to enable
  // the HTTP configuration interface

  auto* networking = new Networking("/system/net", "", "");
  auto* http_server = new HTTPServer();

  auto* digin1 = new DigitalInputCounter(input_pin1, INPUT, RISING, read_delay);
  auto* digin2 = new DigitalInputCounter(input_pin2, INPUT, CHANGE, read_delay);

  auto* scaled1 = new Linear(2, 1, "/digin1/scale");
  auto* scaled2 = new Linear(4, -1, "/digin2/scale");
  digin1->connect_to(scaled1);

  scaled1->connect_to(new LambdaTransform<int, int>([](int input) {
    Serial.printf("millis: %d\n", millis());
    Serial.printf("Counter 1: %d\n", input);
    return input;
  }));

  digin2->connect_to(scaled2)->connect_to(
      new LambdaTransform<int, int>([](int input) {
        Serial.printf("Counter 2: %d\n", input);
        return input;
      }));

  pinMode(output_pin1, OUTPUT);
  app.onRepeat(5,
               []() { digitalWrite(output_pin1, !digitalRead(output_pin1)); });

  pinMode(output_pin2, OUTPUT);
  app.onRepeat(100,
               []() { digitalWrite(output_pin2, !digitalRead(output_pin2)); });
}

// The loop function is called in an endless loop during program execution.
// It simply calls `app.tick()` which will then execute all reactions as needed.
void loop() { app.tick(); }
