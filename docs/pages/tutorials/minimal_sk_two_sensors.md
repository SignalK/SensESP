---
layout: default
title: Minimal Signal K with Two Sensors
parent: Tutorials
nav_order: 6
---

# Tutorial: Sending Data from Two Sensors to Signal K

This Tutorial builds on the [first Tutorial](../minimal_sk) very simply: it adds a second Sensor - AnalogInput. Starting with the `main.cpp` from that first Tutorial, you're going to make only minor changes, described below. There are countless physical sensors whose output can be read by AnalogInput - tank level sensors, temperature sensors, pressure sensors, the list goes on and on. All that matters is that the sensor outputs variable voltage, which the SensESP AnalogInput Sensor will convert to a value from 0 to 1023.

The ESP32 has two Analog-to-Digital Converters (ADC's), but currently, SensESP can access only ADC 1, which allows GPIO's 32 to 36, and 39, to be used for AnalogInput. In this Tutorial, we'll use 36.

In order to use the SensESP Sensor `AnalogInput`, we need to add the `#include` for the header file where that Sensor is defined: `sensesp/sensors/analog_input.h`.

```c++
// Specific to this project
#include "sensesp/sensors/digital_input.h"
#include "sensesp/sensors/analog_input.h"
```

## The Meat of the Program

A SensESP project can read as many physical sensors as you can connect to the ESP. Below, we see the five lines from the first Tutorial, followed immediately by five new, but very similar, lines to read the AnalogInput Sensor.

```c++
  // The lines from the first Tutorial to read GPIO13 with DigitalInputState
  const char* sk_path = "digitalPin.13.state";
  uint8_t pin = 13;
  uint16_t read_delay = 500;
  auto* pin_13_state = new DigitalInputState(pin, INPUT_PULLUP, read_delay);
  pin_13_state->connect_to(new SKOutputBool(sk_path));

  // The new lines to read GPIO36 with AnalogInput
  const char* analog_in_sk_path = "analogPin.36.rawValue";
  uint8_t analog_in_pin = 36;
  uint16_t analog_in_read_delay = 2000;
  auto* analog_in_value = new AnalogInput(analog_in_pin, analog_in_read_delay);
  analog_in_value->connect_to(new SKOutputFloat(analog_in_sk_path));
  ```

The first three lines define variables that will be used by the last two lines, just as we saw in the first Tutorial. Also as in the first Tutorial, the last two lines do the real work:

* `auto* analog_in_value = new AnalogInput(analog_in_pin, analog_in_read_delay);` - this line creates a pointer (called `analog_in_value`) to a AnalogInput Sensor that's going to read GPIO36 every 2000 ms.

* `analog_in_value->connect_to(new SKOutputfloat(analog_in_sk_path));` - this line takes the value of `analog_in_value` (every 2000 ms, as defined above) and passes it along to `SKOutputFloat`, which sends it to Signal K, with the Path defined by `analog_in_sk_path`.

Note that the five new lines above could have been written in only two, by skipping the first three lines and using the values directly, rather than first defining them as variables, like this:

```c++
  auto* analog_in_value = new AnalogInput(36, 2000);
  analog_in_value->connect_to(new SKOutputFloat("analogPin.36.rawValue));
```

If you modify the `main.cpp` that you used for the first Tutorial, by adding the extra lines above, SensESP will do two things:

* Read GPIO13 every 500 ms and send its state to Signal K as `digitalPin.13.state`.
* Read GPIO36 every 2000 ms and send its value to Signal K as `analogPin.36.rawValue`.

## Summary

* SensESP can read multiple physical sensors in a single program.
* All of the boilerplate code remains the same - you simply define more Sensors.

## Related Tutorials

* [The first Tutorial](../minimal_sk), which is the single-sensor version of this Tutorial.
