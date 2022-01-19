---
layout: default
title: Concepts
nav_order: 50
---

# Concepts

## ReactESP

### Basics

[ReactESP](https://github.com/mairas/ReactESP) is an asynchronous programming library for the ESP32 platform.
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

    // create the repeat reaction
    app.onRepeat(interval, toggle_gpio);

    // setup continues
    ...
}
```

What happens in the above example?
We define a callback function `toggle_gpio()`.
Then, we set the pin mode to OUTPUT.
And finally, we create a repeat reaction with the interval `interval`.

Once the program is running, the repeat reaction is triggered automatically every 350 milliseconds.
The interval is calculated from the previous trigger time - if the callback function takes 4 milliseconds to complete, the actual interval still remains 350 ms instead of 354 ms.

Another commonly used and useful time-based reaction is `DelayReaction`.
It triggers after a certain amount of time has passed but does not repeat.
Example use cases for that would be sensor devices in which you trigger the read operation and then come back to get the value after a certain amount of time.
For example, the 1-Wire DS18B20 sensor can take up to 750 ms before the conversion is ready.
In that case, you would first trigger the call and then have something like `app.onDelay(750, read_sensor);` to come back later to read the value.

You can also use `app.onDelay(...)` with a zero delay to trigger the reaction as soon as possible, without blocking the main event loop.

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

    // create the repeat reaction
    app.onRepeat(
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

### Reaction Types

ReactESP is not limited to just delays or repeating reactions.
It also supports the following:

- `StreamReaction`: a reaction that triggers when data is available on a stream, for example on a serial port
- `ISRReaction`: a reaction that is called when an interrupt is triggered (for example, when a GPIO pin is toggled)
- `TickReaction`: a reaction that is called every time the main event loop is executed

### Removing Reactions

All of the `app.onXXX()` calls return a `Reaction` object.
If this object is stored, it can be used to access and manipulate the reaction later.
In practice, you can disable the reaction by calling `reaction->remove()`.
The same reaction can be re-added later by calling `reaction->add()`.

Some attention needs to be paid with `DelayReaction` objects, though.
Since they are by nature one-off operations, the corresponding object is deleted after the reaction is triggered.
You must ensure that you don't try to call the methods of an object that has been deleted, for example by setting a flag in the callback function and checking the flag value before trying to remove the reaction.

## Sensors

A "Sensor" in SensESP is a C++ class that knows how to get data from an actual, physical sensor and pass it along to a Transform (see below). For example, an AnalogInput Sensor knows how to read an AnalogIn pin on the MCU, and a DigitalInputCounter Sensor knows how to read a GPIO pin on the MCU using an interrupt. A BME280value Sensor knows how to read the temperature, humidity, and pressure from a phyical Adafruit BME280 sensor.

The two generic types of sensor data - AnalogInput and DigitalInput (including various versions of DigitalInput, like DigitalInputState and DigitalInputCounter) - are built into the core SensESP code.

Sensors for many specific types of physical sensors, like the BME280, or the SHT31, or the INAxxx, can be implemented by incorporating the relevant sensor library (often from Adafruit) and a few lines of code in `main.cpp`.

Some specific sensors - for example, the MAX31856 thermocouple sensor - require a little more complex programming to read them, so they can be found [here](https://github.com/SensESP). These "add-on" Sensor libraries can easily be included in a SensESP project by adding them to your `platformio.ini` file. If you have a physical sensor that you think fits in this category, and you want some help writing an add-on to read it, make a post in the #sensors channel on the Signalk-dev Slack.

## Transforms

A Transform is a class that takes a value as input (from a Sensor, or from another Transform), does something with that value (transforms it in some way), then outputs the new value, either to another Transform, or to the Signal K Server. In the `rpm_counter.cpp` example referred to above, the meat of the code is these three lines:

```c++
auto* sensor = new DigitalInputCounter(D5, INPUT_PULLUP, RISING, read_delay);

sensor
    ->connectTo(new Frequency(multiplier, config_path_calibrate))
    ->connectTo(new SKOutputFloat(sk_path, config_path_skpath));
```

The first line instantiates a Sensor of type DigitalInputCounter. The second line is a Transform of type Frequency - it takes a raw number from the DigitalInputCounter Sensor and converts it to hertz, then passes it along to SKOutputFloat. SKOutputFloat is a special Transform whose purpose is to send a float value to the Signal K Server.

A much more complex example is `temperature_sender.cpp`, where the meat of the program is this:

```c++
auto* analog_input = new AnalogInput();

analog_input->connectTo(new AnalogVoltage())
    ->connectTo(new VoltageDividerR2(R1, Vin, "/gen/temp/sender"))
    ->connectTo(new TemperatureInterpreter("/gen/temp/curve"))
    ->connectTo(new Linear(1.0, 0.0, "/gen/temp/calibrate"))
    ->connectTo(new SKOutputFloat(sk_path, "/gen/temp/sk"));
```

In this example, there is still only one Sensor (AnalogInput), but several Transforms, all required to turn the raw value from the Analog Input pin on the MCU into a temperature that's sent to the Signal K Server.

You can also include multiple Sensors, each with at least one Transform, in the same program, such as including both of the examples above into the same `main.cpp`, one after the other.

## Configuration Paths

Some Sensors and Transform have parameters that can be configured at run-time. This section explains how to set them up. The user interface to change something at run-time is described [here](../user_interface/index.md). (BAS: this link doesn't work.)

In every case of a configurable value, its value can be set in `main.cpp` with a parameter to the constructor of the Sensor or Transform. By making it configurable, it's easier to make adjustments to your output based on what you're seeing in the real world. For example, the Median Transform is used to smooth output from a "noisy" sensor. By making the "Sample size" configurable, you can experiment with different sample sizes while the MCU is running and outputting data to Signal K, so you can decide when you have the right sample size for your purposes.

Look at the three lines from the `rpm_counter.cpp` example above. There are three constructors - one for the DigitalInputCounter Sensor, one for the Frequency Transform, and one for the SKOutputFloat Transform. The latter two have a configuration path included as the last item in their parameter list, but the first one doesn't. That means that the first one - the DigitalInputCounter - has no values that can be configured "live", but the latter two do. Actually, it's a little more complicated than that, because it's not entirely consistent among all Sensors and Transforms, and it's always optional whether you enable the ability to do the configuration.

Even though a Sensor or Transform has the *ability* to be configurable, it won't *be* configurable unless you provide a configuration path in the constructor when you use it in `main.cpp`. For example,

```c++
auto* analog_input = new AnalogInput();
```

creates an AnalogInput Sensor whose Read Delay will be the default 200 ms, and which can't be configured.

```c++
auto* analog_input = new AnalogInput(250);
```

creates a Sensor with a 250 ms Read Delay that still can't be adjusted in real time, because there is no config_path parameter.

But

```c++
auto* analog_input = new AnalogInput(250, "/analogInput");
```

creates a Sensor with a 250 ms Read Delay that *can* be adjusted in real time, because of the presence of the config_path parameter (`"/analogInput"`).

Your configuration path parameter can be passed with a variable you create, like this:

```c++
const char* sensor_config_path = "/analogInput";
auto* analog_input = new AnalogInput(250, sensor_config_path);
```

or by putting the configuration path string directly into the parameter list of the constructor, like this:

```c++
auto* analog_input = new AnalogInput(250, "/analogInput");
```

### Naming Configuration Paths

The naming of the paths is important, especially when you have multiple Sensors and / or multiple Transforms in your Project, so to be safe, please follow these guidelines:

- Every configuration path name MUST begin with a forward slash.
- Use two levels in your names, so that they look like `"/firstLevel/secondLevel"`, with the first level being a word that groups entries together in a logical manner, and the second level referring to the specific Sensor or Transform that the configuration path relates to. For example:

Two Sensors (one for black water and one for fresh water), each using a Moving Average Transform and outputting to the Signal K Server with SKOutputFloat:

- "/blackWater/analogInput" (for the blackwater AnalogInput() constructor in `main.cpp`)
- "/blackWater/movingAvg" (for the blackwater MovingAverage() constructor in `main.cpp`)
- "/blackWater/skPath" (for the blackwater SKOutputFloat() constructor in `main.cpp`)
- "/freshWater/analogInput" (for the fresh water AnalogInput() constructor)
- "/freshWater/movingAvg" (for the fresh water MovingAverage() constructor)
- "/freshWater/skPath" (for the fresh water SKOutputFloat() constructor)

This will group the configuration entries in the web interface into two groups: "blackWater" and "freshWater". Each group will have three entries: "analogInput", "movingAvg", and "SKOutput". Each "analogInput" entry will have one configurable value: "Read delay"; each "movingAvg" entry will have two configurable values: "Number of samples" and "Multiplier"; and each "skPath" entry will have one configurable value: "SignalK Path".

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
   * SKOutputNumber, which sends the final result to the indicated path on the
   * Signal K server. (Note that each data type has its own version of SKOutput:
   * SKOutputFloat, SKOutputInt, SKOutputBool, and SKOutputString.)
   */
  String sk_path = "navigation.anchor.rodeDeployed";
  String sk_path_config_path = "/rodeDeployed/sk";

  chain_counter->connect_to(accumulator)
      ->connect_to(new SKOutputFloat(sk_path, sk_path_config_path, metadata));
```

If the only metadata you want to provide is the Units, that's most easily done the first of the three ways above. If you want to provide more metadata than just the Units, the last way above is probably best. It's certainly easier to understand than the middle approach above.
