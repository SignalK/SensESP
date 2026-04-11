---
layout: default
title: Concepts
nav_order: 50
---

# Concepts

## How SensESP Works

A SensESP program creates objects in `setup()` and connects them into a processing graph. There is no application logic in `loop()` — just `event_loop()->tick()`. The event loop calls your sensors, runs transforms, and handles networking automatically.

### The three layers

SensESP programs are built from three kinds of objects:

- **Sensors** read physical values (analog pins, I2C devices, serial ports) or generate values programmatically.
- **Transforms** process values: convert units, filter noise, combine multiple inputs, add hysteresis.
- **Outputs** send data to a Signal K server (`SKOutputFloat`, etc.), display it on the status page (`StatusPageItem`), or trigger other actions.

### Connecting the graph

Objects are connected with `connect_to()`:

```c++
sensor->connect_to(transform)->connect_to(sk_output);
```

Data flows from left to right. When the sensor produces a new value, it propagates through the chain automatically.

### The builder

`SensESPAppBuilder` configures WiFi, hostname, OTA, system sensors, and other framework features before `get_app()` finalizes the setup.

### Configuration

Objects with a `config_path` can persist their settings to the file system. Wrapping them in `ConfigItem()` also exposes them in the web configuration UI.

### A minimal complete example

Here is a simplified but complete SensESP program that reads an analog input, applies a linear calibration, and sends the result to a Signal K server:

```c++
#include "sensesp_app_builder.h"
#include "sensesp/sensors/analog_input.h"
#include "sensesp/transforms/linear.h"
#include "sensesp/signalk/signalk_output.h"

using namespace sensesp;

void setup() {
  SetupLogging();

  SensESPAppBuilder builder;
  builder.set_hostname("my-sensesp-device")
         ->get_app();

  auto* analog_input = new AnalogInput(36, 1000);

  analog_input
      ->connect_to(new Linear(1.0, 0.0, "/calibration"))
      ->connect_to(new SKOutputFloat("sensors.indoor.illuminance"));

  ConfigItem(analog_input)
      ->set_title("Analog Input")
      ->set_description("Read interval for the analog input.")
      ->set_sort_order(1000);
}

void loop() { event_loop()->tick(); }
```

Walking through the code:

- `SetupLogging()` initializes serial debug output.
- `SensESPAppBuilder` sets up WiFi, mDNS, the web UI, and the Signal K connection. `get_app()` finalizes that configuration.
- `AnalogInput(36, 1000)` reads GPIO 36 every 1000 ms.
- `Linear(1.0, 0.0, "/calibration")` applies a y = ax + b transform. The `"/calibration"` config path lets you adjust the slope and offset at runtime.
- `SKOutputFloat(...)` sends the result to the Signal K server at the given path.
- `ConfigItem(analog_input)` exposes the analog input's settings (read interval) in the web configuration UI.
- `loop()` just ticks the event loop — all the real work happens in the objects you created in `setup()`.

## ReactESP

### Basics

[ReactESP](https://github.com/mairas/ReactESP) is an asynchronous programming and event loop library for the ESP32 platform.
SensESP builds on top of ReactESP and uses it extensively under the hood.
If you want to build more complex programs on top of SensESP or want to hack on SensESP internals, some basic understanding on the ReactESP basic concepts is highly useful.

First, an example:

```c++
...
// define the GPIO pin to toggle
const int kGpioPin = 15;
// define the repeat interval in milliseconds
const int interval = 350;

// define the callback function
void toggle_gpio() {
    bool current_state = digitalRead(kGpioPin);
    digitalWrite(kGpioPin, !current_state);
}

void setup() {
    // other initialization first
    ...

    // initialize the GPIO pin
    pinMode(kGpioPin, OUTPUT);

    // create the repeat event
    event_loop()->onRepeat(interval, toggle_gpio);

    // setup continues
    ...
}
```

What happens in the above example?
We define a callback function `toggle_gpio()`.
Then, we set the pin mode to OUTPUT.
And finally, we create a repeat event with the interval `interval`.

Once the program is running, the repeat event is triggered automatically every 350 milliseconds.
The interval is calculated from the previous trigger time - if the callback function takes 4 milliseconds to complete, the actual interval still remains 350 ms instead of 354 ms.

Another commonly used and useful time-based event is `DelayEvent`.
It triggers after a certain amount of time has passed but does not repeat.
Example use cases for that would be sensor devices in which you trigger the read operation and then come back to get the value after a certain amount of time.
For example, the 1-Wire DS18B20 sensor can take up to 750 ms before the conversion is ready.
In that case, you would first trigger the call and then have something like `event_loop()->onDelay(750, read_sensor);` to come back later to read the value.

You can also use `event_loop()->onDelay(...)` with a zero delay to trigger the event as soon as possible, without blocking the main event loop.

### Lambdas

It is common to implement the callback function as a [lambda expression](https://en.cppreference.com/w/cpp/language/lambda).
Lambdas are unnamed functions that have a special power that they can "capture" variables from the surrounding scope.

Let's rewrite our GPIO toggling example as a lambda:

```c++
...

void setup() {
    // other initialization first
    ...

    // initialize the GPIO pin
    pinMode(kGpioPin, OUTPUT);

    // create the repeat event
    event_loop()->onRepeat(
        interval,
        []() {
            bool current_state = digitalRead(kGpioPin);
            digitalWrite(kGpioPin, !current_state);
        }
    );

    // setup continues
    ...
}

```

Now, instead of having the `toggle_gpio()` function defined somewhere else in the file, we define the callback right where it is used.
The brackets `[]` define the start of the lambda expression.
They may also contain definitions for variable capture.
To learn more about that topic, see the [cppreference.com discussion and examples](https://en.cppreference.com/w/cpp/language/lambda).

### Event Types

ReactESP is not limited to just delays or repeating events.
It also supports the following:

- `StreamEvent`: an event that triggers when data is available on a stream, for example on a serial port
- `ISREvent`: an event that is called when an interrupt is triggered (for example, when a GPIO pin is toggled)
- `TickEvent`: an event that is called every time the main event loop is executed

### Removing Events

All of the `event_loop()->onXXX()` calls return a `Event` object.
If this object is stored, it can be used to access and manipulate the event later.
In practice, you can disable the event by calling `event->remove()`.
The same event can be re-added later by calling `event->add()`.

Some attention needs to be paid with `DelayEvent` objects, though.
Since they are by nature one-off operations, the corresponding object is deleted after the event is triggered.
You must ensure that you don't try to call the methods of an object that has been deleted, for example by setting a flag in the callback function and checking the flag value before trying to remove the event.

## Sensors

A "Sensor" in SensESP is a [C++ class](https://signalk.org/SensESP/generated/docs/classsensesp_1_1_sensor_t.html) that knows how to get data from an actual, physical sensor and pass it along to a Transform (see below).
For example, an AnalogInput Sensor knows how to read an AnalogIn pin on the MCU, and a [DigitalInputCounter](https://signalk.org/SensESP/generated/docs/classsensesp_1_1_digital_input_counter.html) Sensor knows how to read a GPIO pin on the MCU using an interrupt.

The two generic types of sensor data - [AnalogInput](https://signalk.org/SensESP/generated/docs/classsensesp_1_1_analog_input.html) and [DigitalInput](https://signalk.org/SensESP/generated/docs/classsensesp_1_1_digital_input.html) (including various versions of DigitalInput, like [DigitalInputState](https://signalk.org/SensESP/generated/docs/classsensesp_1_1_digital_input_state.html) and [DigitalInputCounter](https://signalk.org/SensESP/generated/docs/classsensesp_1_1_digital_input_counter.html)) - are built into the core SensESP code.

Sensors for many specific types of physical sensors, like the BME280, or the SHT31, or the INAxxx, can be implemented by incorporating the relevant sensor library (often from Adafruit) and a few lines of code in `main.cpp`.

Some specific sensors - for example, the MAX31856 thermocouple sensor - require a little more complex programming to read them, so they can be found [here](../additional_resources/). These "add-on" Sensor libraries can easily be included in a SensESP project by adding them to your `platformio.ini` file. If you have a physical sensor that you think fits in this category, and you want some help writing an add-on to read it, make a post in the #sensesp channel on the Signal K Discord server.

## Transforms

A [Transform](https://signalk.org/SensESP/generated/docs/classsensesp_1_1_transform.html) is a class that takes a value as input (from a Sensor, or from another Transform), does something with that value (transforms it in some way), then outputs the new value, either to another Transform, or to the Signal K Server. In the `rpm_counter.cpp` example (found in the `examples/` directory), the meat of the code is these three lines:

```c++
auto* sensor = new DigitalInputCounter(D5, INPUT_PULLUP, RISING, read_delay);

sensor
    ->connect_to(new Frequency(multiplier, config_path_calibrate))
    ->connect_to(new SKOutputFloat(sk_path, config_path_skpath));
```

The first line instantiates a Sensor of type DigitalInputCounter. The second line is a Transform of type [Frequency](https://signalk.org/SensESP/generated/docs/classsensesp_1_1_frequency.html) - it takes a raw number from the DigitalInputCounter Sensor and converts it to hertz, then passes it along to [SKOutputFloat](https://signalk.org/SensESP/generated/docs/classsensesp_1_1_s_k_output_numeric.html). SKOutputFloat is a special Transform whose purpose is to send a float value to the Signal K Server.

A much more complex example is `temperature_sender.cpp`, where the meat of the program is this:

```c++
auto* analog_input = new AnalogInput();

analog_input->connect_to(new AnalogVoltage())
    ->connect_to(new VoltageDividerR2(R1, Vin, "/gen/temp/sender"))
    ->connect_to(new TemperatureInterpreter("/gen/temp/curve"))
    ->connect_to(new Linear(1.0, 0.0, "/gen/temp/calibrate"))
    ->connect_to(new SKOutputFloat(sk_path, "/gen/temp/sk"));
```

In this example, there is still only one Sensor (AnalogInput), but several Transforms, all required to turn the raw value from the Analog Input pin on the MCU into a temperature that's sent to the Signal K Server.

You can also include multiple Sensors, each with at least one Transform, in the same program, such as including both of the examples above into the same `main.cpp`, one after the other.

### Additional Transforms

SensESP v3 introduces several new transforms for common data processing tasks:

**Filter** — Conditionally passes values based on a predicate function. Values that don't pass the test are silently dropped.

```c++
// Only pass through temperatures above freezing
auto* above_freezing = new Filter<float>([](float t) { return t > 273.15; });
sensor->connect_to(above_freezing)->connect_to(sk_output);
```

**Throttle** — Limits the rate of output updates. If values arrive faster than the minimum interval, excess values are dropped (not averaged).

```c++
// Emit at most once every 5 seconds
sensor->connect_to(new Throttle<float>(5000))->connect_to(sk_output);
```

**Repeat and variants** — Re-emit the last received value at a regular interval, useful for systems that expect continuous updates.

- `Repeat<T>` — re-emits the last value at a given interval
- `RepeatStopping<T>` — stops repeating if no new input arrives within `max_age`
- `RepeatExpiring<T>` — emits a `Nullable<T>` that becomes invalid after `max_age`, signaling that the value is stale
- `RepeatConstantRate<T>` — emits at a constant rate regardless of input timing, also using `Nullable<T>`

```c++
// Re-emit temperature every 2 seconds, expire after 10 seconds without update
sensor->connect_to(new RepeatExpiring<float>(2000, 10000))
      ->connect_to(sk_output);
```

**Nullable&lt;T&gt;** — A wrapper type that can represent an invalid or expired value. Used by `RepeatExpiring` and `RepeatConstantRate` to signal stale data. When a `Nullable<T>` is invalid, it serializes to JSON `null`.

**Hysteresis** — A threshold switch with a dead zone to prevent oscillation. Useful for on/off control like pump activation.

```c++
// Turn on when temperature exceeds 350K, turn off when it drops below 340K
auto* overheat = new Hysteresis<float, bool>(340.0, 350.0, false, true);
sensor->connect_to(overheat)->connect_to(alarm_output);
```

**Join and Zip** — Combine multiple input values into a tuple.

- `Join<T1, T2>` emits a tuple whenever *any* input is updated (as long as all values are within `max_age`)
- `Zip<T1, T2>` emits a tuple only when *all* inputs have been updated

Since these transforms accept multiple inputs, they expose individual consumer members rather than accepting input directly. Connect producers to the consumers using `std::get<N>()`:

```c++
auto* join = new Join<float, float>();

temperature_sensor->connect_to(&std::get<0>(join->consumers));
pressure_sensor->connect_to(&std::get<1>(join->consumers));

// join now produces std::tuple<float, float>
join->connect_to(new LambdaTransform<std::tuple<float, float>, float>(
    [](std::tuple<float, float> input) -> float {
      auto [temp, pressure] = input;
      return calculate_air_density(temp, pressure);
    }
));
```

Variants `Join3`, `Join4`, `Join5` and `Zip3`, `Zip4`, `Zip5` support up to five inputs.

For complete working examples, see the `examples/join_and_zip.cpp`, `examples/repeat_transform.cpp`, and `examples/hysteresis.cpp` files in the SensESP repository.

## Configuration Paths

Configuration paths allow objects to persist their configuration to the device file system. Having a config_path defined also is a requirement for exposing the object to the web interface.

Many Sensors and Transforms have parameters that can be configured at run-time. This section explains how to set them up.

Even though a Sensor or Transform has the *ability* to be configurable, it won't *be* configurable unless you provide a configuration path in the constructor when you use it in `main.cpp` and call `ConfigItem` on it. For example,

```c++
auto* analog_input = new AnalogInput();
```

creates an AnalogInput Sensor whose Read Delay will be the default 200 ms, and which can't be configured.

```c++
auto* analog_input = new AnalogInput(250);
```

creates a Sensor with a 250 ms Read Delay that still can't be adjusted in real time, because there is no config_path parameter.

Defining a config_path parameter allows the object to save its configuration to the device file system:

```c++
auto* analog_input = new AnalogInput(250, "/analogInput");
```

Your configuration path parameter can also be passed with a variable you create, like this:

```c++
const char* sensor_config_path = "/analogInput";
auto* analog_input = new AnalogInput(250, sensor_config_path);
```

Now, if you want to expose the object to the web interface, call `ConfigItem` on it:

```c++
ConfigItem(analog_input)
  ->set_title("Analogue Input")
  ->set_description("Analog input read interval adjustment.")
  ->set_sort_order(1100);
```

That adds the created `analog_input object` to the web UI.

The config_path is used only for saving the configuration to the device file system. The paths must be unique and start with a forward slash.

## Signal K Paths

All data that you send to a Signal K Server must have a "path" that will look something like this: `environment.outside.temperature`. The [Signal K Specification](https://signalk.org/specification/1.5.0/doc/vesselsBranch.html) includes a long list of paths that have already been defined. You should use one of them if you can find one that fits your data, because the various programs that "consume" Signal K data (such as Instrument Panel, KIP, and Wilhelm SK) will know how to properly display many of them. For example, they may know that `environment.outside.temperature` is a temperature, and will offer appropriate instruments to display a temperature.

But not all data you might send will be in the Specification. In that case, you should find a path that's close, and then modify it to fit your data. For example, the Specification has no reference to either "bilge" or "pump", but if you want to send the state of your bilge pump to Signal K, it needs a path. You could use something like `electrical.bilgePump.state`, or `electrical.pump.bilge.state`. It's not all that important what you come up with, as long as it's descriptive, so you'll know exactly what it is when you see the data in Signal K.

If you do create a path for any data you're sending out of SensESP, you'll want to provide the appropriate unit of measurement for that data, if there is one. See below for details.

## Signal K Output and Metadata

SensESP can send data to Signal K as any of these data types, using (this particular Transform).

- Float (SKOutputFloat)
- Integer (SKOutputInt)
- Boolean (SKOutputBool)
- String (SKOutputString)

You can see two examples of a float in the Transforms(#transforms) section earlier in this page.

Part of the [Signal K Specification](https://signalk.org/specification/1.5.0/doc/vesselsBranch.html) for many of the [paths](#signal-k-paths) is the "Units" for that type of data. For example, electrical paths use volts, amperes, Joules, etc. All temperatures are in degrees Kelvin, revolutions and frequency are in Hertz, and pressure is in Pascals. All of these units are the [SI Units](https://en.wikipedia.org/wiki/International_System_of_Units) for the particular type of data if one exists in the Standard. Don't worry if you're not used to working with degrees Kelvin, for example - the "consumers" of Signal K data offer ways for you to convert to Fahrenheit or Celsius, or whatever the typical units are for each type of data.

If you don't find a particular path in the Signal K Specification and you create your own, you may also want to tell the Signal K Server what Unit you would like for that path. (But only for Floats and Integers - there is no SI Unit for a Bool or a String.) This is valuable so that the final consumers of the data (Instrument Panel, etc.) will know how best to display that data.

Here are three different ways to specify the Units for any data you're sending to Signal K:

```c++
// Specify only the units, using a string ("V" for volts) as the third parameter to SKOoutput<type>:
->connect_to(new SKOutputFloat(sk_path, sk_config_path, "V"));

// Make the third parameter a new SKMetadata object, but pass only a string to its constructor:
->connect_to(new SKOutputFloat(sk_path, sk_config_path, new SKMetadata("V")));

// From the `chain_counter.cpp` example:
/**
   * There is no path for the amount of anchor rode deployed in the current
   * Signal K specification. By creating an instance of SKMetaData, we can send
   * a partial or full definition of the metadata that other consumers of Signal
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
   * SKOutputFloat, which sends the final result to the indicated path on the
   * Signal K server. (Note that each data type has its own version of SKOutput:
   * SKOutputFloat, SKOutputInt, SKOutputBool, and SKOutputString.)
   */
  String sk_path = "navigation.anchor.rodeDeployed";
  String sk_path_config_path = "/rodeDeployed/sk";

  chain_counter->connect_to(accumulator)
      ->connect_to(new SKOutputFloat(sk_path, sk_path_config_path, metadata));
```

If the only metadata you want to provide is the Units, that's most easily done the first of the three ways above. If you want to provide more metadata than just the Units, the last way above is probably best. It's certainly easier to understand than the middle approach above.
