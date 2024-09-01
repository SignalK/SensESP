/**
 * @file repeat_transform.cpp
 * @brief Example of different Repeat transforms.
 *
 * Repeat transforms transmit their input value at regular intervals, ensuring
 * output even at the absence of input.
 *
 * Try running this code with the serial monitor open ("Upload and Monitor"
 * in PlatformIO menu). The program will produce capital letters every second,
 * lowercase letters every 3 seconds, and integers every 10 seconds. Comment
 * out and enable different Repeat transform variants to see how they affect the
 * output.
 *
 */

#include "sensesp.h"

#include <math.h>

#include "sensesp/sensors/sensor.h"
#include "sensesp/system/lambda_consumer.h"
#include "sensesp/transforms/repeat.h"
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

  // Repeat the values every 2 seconds

  auto repeat_A = new Repeat<char>(2000);
  auto repeat_a = new Repeat<char>(2000);
  auto repeat_int = new Repeat<int>(2000);

  // Pay attention to the individual columns of the program console output.
  // Capital letters are produced every second. Repeat gets always triggered as
  // a result, but never on its own because the repeat timer always is reset
  // before it expires. Lowercase letters are produced every 3 seconds. Repeat
  // gets triggered immediately and then again after 2 seconds. Integers are
  // produced every 10 seconds. Repeat gets triggered immediately and then
  // again after every 2 seconds until the next integer is produced.

  // Try commenting out the Repeat lines above and uncommenting the
  // RepeatStopping lines below.

  // auto repeat_A = new RepeatStopping<char>(2000, 5000);
  // auto repeat_a = new RepeatStopping<char>(2000, 5000);
  // auto repeat_int = new RepeatStopping<int>(2000, 5000);

  // The maximum age is set to 5 seconds. Both the capital and lowercase
  // letters are produced like before because their repetition rates are
  // faster than the expiration time. However, the integers are produced
  // only every 10 seconds, and they stop being repeated after 5 seconds
  // until a new integer is produced.

  // auto repeat_A = new RepeatExpiring<char>(2000, 5000, '?');
  // auto repeat_a = new RepeatExpiring<char>(2000, 5000, '?');
  // auto repeat_int = new RepeatExpiring<int>(2000, 5000, -1);

  // The expiration time is set to 5 seconds. Both the capital and lowercase
  // letters are produced like before because their repetition rates are
  // faster than the expiration time. However, the integers are produced
  // only every 10 seconds, and the value does expire after 5 seconds, indicated
  // by the -1 value that gets output after the expiry, until a new integer is
  // produced.

  // Finally, try commenting out the RepeatExpiring lines above and uncommenting
  // the RepeatConstantRate lines below.

  // auto repeat_A = new RepeatConstantRate<char>(2000, 5000, '?');
  // auto repeat_a = new RepeatConstantRate<char>(2000, 5000, '?');
  // auto repeat_int = new RepeatConstantRate<int>(2000, 5000, -1);

  // Notice how the repetitions are no longer triggered by the sensors but
  // are produced at a constant rate, in clusters. The letters still
  // never expire, but the integers do.

  sensor_A->connect_to(repeat_A);
  sensor_a->connect_to(repeat_a);
  sensor_int->connect_to(repeat_int);

  repeat_A->connect_to(new LambdaConsumer<char>(
      [](char value) { ESP_LOGD("App", "Repeat: %c", value); }));

  repeat_a->connect_to(new LambdaConsumer<char>(
      [](char value) { ESP_LOGD("App", "Repeat:   %c", value); }));

  repeat_int->connect_to(new LambdaConsumer<int>(
      [](int value) { ESP_LOGD("App", "Repeat:     %d", value); }));
}

void loop() {
  // We're storing the event loop in a static variable so that it's only
  // acquired once. Saves a few function calls per loop iteration.
  static auto event_loop = SensESPBaseApp::get_event_loop();
  event_loop->tick();
}
