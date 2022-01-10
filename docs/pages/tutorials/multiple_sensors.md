---
layout: default
title: Multiple External Sensors
parent: Tutorials
nav_order: 22
---

# Tutorial: Reading Multiple Sensors

The previous tutorials have illustrated how to read a number of different types of sensors:

* The ones in the SensESP core code that use the native Arduino functions `AnalogRead()` and `DigitalRead()`.
* The ones for which some SensESP-specific code is necessary to work easily with SensESP.
* The ones that are basic Arduino-compatible sensors with very simple methods for reading their values.

This Tutorial will illustrate how to read values from more than one sensor in the same SensESP project, regardless of which of the above types they are. The example will read one of each of the three types listed above:

* DigitalInputState (using SensESP core code)
* 1-Wire Temperature (using a special library written just for SensESP)
* BMP280 Pressure (using the "generic sensor" approach)

Each of these has been used in a previous Tutorial, and the code included below comes directly from those three tutorials. A link to each of them is provided at the end.

## platformio.ini

This is the `lib_deps` section of the `platformio.ini`.

```c++
lib_deps =
  SignalK/SensESP @ ^ 2.0.0
  https://github.com/SensESP/OneWire
  adafruit/Adafruit BMP280 Library @ ^ 2.6.0
```

The first line says "include the SensESP library, any version 2.0.0 or higher". Since DigitalInputState is a Sensor defined in the SensESP core code, this line covers that Sensor.

The second line includes the external library that has been written to make the 1-Wire protocol work with SensESP. It's not recognized by PlatformIO, so we have to include it by its GitHub URL.

The last line includes the Adafruit BMP280 library (version 2.6.0 or higher) for the BMP280 Pressure sensor.

## main.cpp

Here we've included an entire `main.cpp` that reads values from all three sensors. Lines that are "boilerplate" are marked with a comment starting with "BP", so you can ignore them - they're the same as in every other Tutorial.

```c++
// BP #includes:
#include <Arduino.h>
#include "sensesp_app.h"
#include "sensesp_app_builder.h"
#include "sensesp/sensors/sensor.h"
#include "sensesp/signalk/signalk_output.h"

// Sensor-specific #includes
#include "sensesp/sensors/digital_input.h"
#include "sensesp_onewire/onewire_temperature.h" (@mairas: I copied this from the SensESP/OneWire repo example: is is still correct?)
#include <Adafruit_BMP280.h>
#include <Wire.h>

// BP
using namespace sensesp;

// BP
reactesp::ReactESP app;

// Specific to the BMP280:
Adafruit_BMP280 bmp280;
float read_pressure_callback() { return bmp280.readPressure(); }

// BP
void setup() {
#ifndef SERIAL_DEBUG_DISABLED
  SetupSerialDebug(115200);
#endif
  SensESPAppBuilder builder;
  sensesp_app = builder.get_app();

  // Code from the DigitalInputState Tutorial main.cpp, with minor changes:
  uint8_t digital_input_pin = 13;
  uint16_t digital_input_read_delay = 500;
  auto* pin_13_state = 
      new DigitalInputState(digital_input_pin, INPUT_PULLUP, digital_input_read_delay);
  pin_13_state->connect_to(new SKOutputBool("digitalPin.13.state"));
  
  // Code from the 1-Wire Tutorial main.cpp, with minor changes:
  uint8_t one_wire_pin = 4;
  DallasTemperatureSensors* dts = new DallasTemperatureSensors(one_wire_pin);
  uint one_wire_read_delay = 500;
  auto* coolant_temp =
      new OneWireTemperature(dts, one_wire_read_delay);
  coolant_temp
      ->connect_to(new SKOutputFloat("propulsion.mainEngine.coolantTemperature");
  auto* exhaust_temp =
      new OneWireTemperature(dts, one_wire_read_delay);
  exhaust_temp
      ->connect_to(new SKOutputFloat("propulsion.mainEngine.exhaustTemperature"));    

  // Code from the BMP280 Tutorial main.cpp, with minor changes:
  bmp280.begin();
  auto* engine_room_temp =
      new RepeatSensor<float>(read_interval, read_pressure_callback);
  engine_room_temp
      ->connect_to(new SKOutputFloat("propulsion.engineRoom.temperature"));

   // BP
  sensesp_app->start();
}

// BP
void loop() {
  app.tick();
}
```

## Summary

* You can read as many physical sensors, and read as many values from each sensor, as you like.
* The boilerplate is exactly the same, regardless of how many sensors and values you're reading.

## Related Tutorials

* The [DigitalInputState Tutorial](../minimal_sk/#the-meat-of-the-program)
* The [1-Wire Tutorial](../one_wire)
* The [BMP280 Tutorial](../bmp280)