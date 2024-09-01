/**
 * @file join_and_zip.cpp
 * @brief Example of Join and Zip transforms.
 *
 * Join and Zip transforms combine multiple input values into a single output.
 *
 * Try running this code with the serial monitor open ("Upload and Monitor"
 * in PlatformIO menu). The program will produce capital letters every second,
 * lowercase letters every 3 seconds, and integers every 10 seconds. Comment
 * out and enable Join and zip transforms to see how they affect the
 * output.
 *
 */

#include "sensesp.h"

#include <math.h>

#include "sensesp/sensors/sensor.h"
#include "sensesp/transforms/join.h"
#include "sensesp/transforms/lambda_transform.h"
#include "sensesp/transforms/zip.h"
#include "sensesp_minimal_app_builder.h"

using namespace sensesp;

SensESPMinimalApp* sensesp_app;

void setup() {
  SetupLogging();

  // Note: SensESPMinimalAppBuilder is used to build the app. This creates
  // a minimal app with no networking or other bells and whistles which
  // would be distracting in this example. In normal use, this is not what
  // you would use. Unless, of course, you know that is what you want.
  SensESPMinimalAppBuilder builder;

  sensesp_app = builder.get_app();

  // Produce capital letters every second
  auto sensor_A = new RepeatSensor<char>(1000, []() {
    static char value = 'Z';
    if (value == 'Z') {
      value = 'A';
    } else {
      value += 1;
    }
    return value;
  });

  sensor_A->connect_to(new LambdaConsumer<char>(
      [](char value) { ESP_LOGD("App", "        %c", value); }));

  // Produce lowercase letters every 3 seconds
  auto sensor_a = new RepeatSensor<char>(3000, []() {
    static char value = 'z';
    if (value == 'z') {
      value = 'a';
    } else {
      value += 1;
    }
    return value;
  });

  sensor_a->connect_to(new LambdaConsumer<char>(
      [](char value) { ESP_LOGD("App", "          %c", value); }));

  // Produce integers every 10 seconds
  auto sensor_int = new RepeatSensor<int>(10000, []() {
    static int value = 0;
    value += 1;
    return value;
  });

  sensor_int->connect_to(new LambdaConsumer<int>(
      [](int value) { ESP_LOGD("App", "            %d", value); }));

  // Join the three producer outputs into one tuple. A tuple is a data
  // structure that can hold multiple values of different types. The resulting
  // tuple can be consumed by consumers to process the values together.

  auto* merged = new Join3<char, char, int>(5000);

  // The Join transform will emit a tuple whenever any of the producers emit a
  // new value, as long as all values are less than max_age milliseconds old.

  // Once an integer is produced, the Join transform produces tuples for all
  // new letter input until the last integer value is over 5000 milliseconds
  // old.

  // Next, try commenting out the Join transform and enabling the Zip transform
  // below to see how it affects the output.

  // auto* merged = new Zip3<char, char, int>(5000);

  // The Zip transform will emit a tuple only when all producers have emitted a
  // new value within max_age milliseconds. This has the effect of synchronizing
  // the producers' outputs, at the cost of potentially waiting for all
  // producers to emit a new value.

  // Below, the sensors are connected to the consumers of the Join/Zip
  // transform. The syntax here is a bit more complex and warrants some
  // explanation.

  // `merged` is our variable holding a pointer to the Join or Zip transform.
  // The `consumers` member of the transform is a tuple of LambdaConsumers
  // that consume and process the values of the producers. Subscripts [] can
  // only be used to access elements of a same type, but our LambdaConsumers
  // are of potentially different types - hence the tuple. The `std::get<>()`
  // function is used to access the elements of the tuple. The first argument
  // is the index of the element in the tuple, starting from 0.

  // `connect_to()` expects a pointer to a `ValueConsumer`, but `std::get`
  // returns a reference to the tuple element. The `&` operator is used to
  // get the address of the tuple element, which is then passed to
  // `connect_to()`.

  // TL;DR: We connect each sensor to the corresponding consumer Join or
  // Zip transform.

  sensor_A->connect_to(&(std::get<0>(merged->consumers)));
  sensor_a->connect_to(&(std::get<1>(merged->consumers)));
  sensor_int->connect_to(&(std::get<2>(merged->consumers)));

  // Here, we have a LambdaTransform that takes the tuple of values produced
  // by the Join/Zip transform and converts it into a string. Note the template
  // arguments: the transform input is a tuple of char, char, and int, and the
  // output is a String. The same input type needs to be defined in our lambda
  // function, starting with [].

  auto merged_string = new LambdaTransform<std::tuple<char, char, int>, String>(
      [](std::tuple<char, char, int> values) {
        return String(std::get<0>(values)) + " " + String(std::get<1>(values)) +
               " " + String(std::get<2>(values));
      });

  // Remember to connect the Join/Zip transform to the LambdaTransform:

  merged->connect_to(merged_string);

  // Finally, we connect the LambdaTransform to a consumer that will print the
  // merged values to the console.

  merged_string->connect_to(new LambdaConsumer<String>(
      [](String value) {
        ESP_LOGD("App", "Merged: %s", value.c_str());
      }));
}

void loop() {
  // We're storing the event loop in a static variable so that it's only
  // acquired once. Saves a few function calls per loop iteration.
  static auto event_loop = SensESPBaseApp::get_event_loop();
  event_loop->tick();
}
