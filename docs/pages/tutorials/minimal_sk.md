---
layout: default
title: Minimal Signal K Example
parent: Tutorials
nav_order: 5
---

# First Tutorial: Minimal Example of Sending Data to Signal K

## Audience: Absolute Beginner

This Tutorial is written for someone who has never seen SensESP before, someone who may not be very familiar with Signal K, and who may not even have a lot of C++ experience. (Although if you have NO C++ experience, this still may be a stretch for you - sorry.) It explains literally every line in a basic SensESP program. But this is the only Tutorial that does that - all other Tutorials will assume you've been through this one, or that you have enough experience to figure out the parts that aren't explained.

## Objectives

By far the most common use case for SensESP is reading data from some kind of sensor and sending it to Signal K. This first Tutorial will not only show how to do that, but will also explain every line in the program file (`main.cpp`) and some of the lines in the configuration file (`platformio.ini`).

This very basic example reads the state of GPIO pin #13 every 500 ms and sends it to your Signal K Server[^1]. You can toggle the state of the pin by connecting a jumper wire to it and touching the other end to a GND pin, then releasing it. The pin's state will toggle between TRUE and FALSE, and you can see that in the Signal K Data Browser.

## Downloading and Opening the Files

The files for this Tutorial are [in this GitHub repository](https://github.com/SensESP/Tutorial-Minimal-SignalK). The steps below will download and open them with PlatformIO.

1. Start Visual Studio Code[^2].
2. On the VS Code welcome page, click on "Clone Git Repository".
3. Copy-paste this URL into the field at the top of the page, then click "Clone from URL": `https://github.com/SensESP/Tutorial-Minimal-SignalK`.
4. Browse to the folder that will be the *parent* of this repo (such as `/Documents/MyGitHubProjects`), then click on the "Select Repository Location" button.
5. Click the "Open" button in the lower right corner.
6. The Project will open and `platformio.ini` will be open in the main Editor window. At the top of that window, you'll see a tab with "*platformio.ini*", in italics. Italics means the file is open, but as soon as you open another file, this one will close. So double-click on that tab, and the italics will change to normal text, indicating that the file will stay open when you open another file.
7. In the "Explorer" panel on the left, click on `> src`, then double-click on `main.cpp`. You now have open the only two files you'll need for this Tutorial.

## A Brief Overview of `platformio.ini`

Every PlatformIO project needs a `platformio.ini` file. The one you just downloaded is for SensESP running on an ESP32. It contains the settings that PlatformIO needs to build this project and upload it to the ESP32. Feel free to browse through the file, but it's unlikely that you'll ever have to change anything, except adding libraries for new sensors, which is covered in other Tutorials.

The one section that's important for this Tutorial is:

```c++
lib_deps =
   SignalK/SensESP @ ^2.0.0
```

`lib_deps` (Library Dependencies) lists every library that PlatformIO needs to build this project. Because this project uses only code that's in the SensESP core, the only library needed is `Signalk/SensESP @ ^2.0.0` (version 2.0.0 or higher).

SensESP uses a lot of other libraries, but PlatformIO will automatically download them.

## A Tour of `main.cpp`

In PlatformIO, the main program file is always called `main.cpp`. The `main.cpp` for every SensESP project is going to have a lot of the same lines in it - the lines of "boilerplate" ("BP:") that make it a SensESP program. These are all described below. Pay attention: they won't be explained again in any other Tutorial.

### The Boilerplate

```c++
// BP:
#include "sensesp_app_builder.h"
#include "sensesp/signalk/signalk_output.h"

// Specific to this project
#include "sensesp/sensors/digital_input.h"
```

Every SensESP program that sends data to SignalK will contain the first two `#includes` above. After that, you `#include` any other files needed for your particular project. Since we're using only one SensESP Sensor (one of the ones defined in `/sensesp/sensors/digital_input.h`), there is only one project-specific `#include`. Other Tutorials will highlight the `#includes` they need.

```c++
using namespace sensesp;
```

Every SensESP program will have this line. If you're not familiar with the "namespace" concept, don't worry - it just needs to be there.

```c++
// BP:
reactesp::ReactESP app;
```

ReactESP is an application framework that was developed, in part, for SensESP. You can read all about it [here](https://github.com/mairas/ReactESP#reactesp) if you like, but you don't have to. Just understand that it functions like an event scheduler and executor. Within a ReactESP app, you define one or more events to occur, and then you start them. An example (explained in detail later) is what we're doing in this Tutorial: "read the state of GPIO13 every 500 ms and send it to Signal K". You can have one or many events defined in a single SensESP program.

```c++
// BP:
void setup() {
```

SensESP's `main.cpp` is structured like an Arduino sketch, with `setup()` and `loop()` functions. Inside `setup()`, you define all the ReactESP "events", and then start them at the end. All of the important stuff in a SensESP program happens either before, or inside of, `setup()`.

```c++
// BP:
#ifndef SERIAL_DEBUG_DISABLED
  SetupSerialDebug(115200);
#endif
```

SensESP, as well as many of the libraries it uses, can send messages to the PlatformIO Serial Monitor. These three lines are what enables that.

```c++
  // BP:
  SensESPAppBuilder builder;
  sensesp_app = builder.get_app();
```

Every SensESP program has an instance of the C++ Class `SensESPApp` called `sensesp_app`. These two lines create it. It's what handles all of the wifi and Signal K stuff, so you don't have to think about it.

After the `sensesp_app = builder.get_app();` line comes the real meat of the program - setting up the "event(s)" that the program will execute over and over. Those are the lines that make each program different, and they'll be described in detail for each of the Tutorials. But there's one last section of boilerplate to describe:

```c++
  // BP:
  sensesp_app->start();
}

// BP:
void loop() { app.tick(); }

```

The lines above should be at the end of every SensESP program. The first one, `sensesp_app->start();`, tells the ReactESP framework that all events have been defined, so it's time to start them all.

The last line defines the `loop()` function, which, in the ReactESP framework, looks for an "event" to execute at every tick of the microprocessor's clock.

### The Meat of the Program

As noted above, the "meat" of this particular Tutorial is setting up the event that's going to read the state of GPIO13 every 500 ms and send it to Signal K. It's done with the following code:

```c++
  // 1. Set the Signal K Path for the output
  const char* sk_path = "digitalPin.13.state";

  // 2. Define the GPIO pin to read
  uint8_t pin = 13;

  // 3. Define how often to read the state of the pin, in ms
  uint16_t read_delay = 500;

  // 4. Create an instance of a DigitalInputState sensor and define
  //    how often it will be read.
  auto* pin_13_state = new DigitalInputState(pin, INPUT_PULLUP, read_delay);

  // 5. Send the state of the pin to the Signal K server as a Boolean
  pin_13_state->connect_to(new SKOutputBool(sk_path));
  ```

The first three lines define variables that will be used by the last two lines:

1. `sk_path` is like a label that Signal K will use to identify this data once it's in Signal K. (This is a Signal K term, not a SensESP term.) There a bunch of them defined in the [Signal K Specification Appendix A](https://signalk.org/specification/1.5.0/doc/vesselsBranch.html), but here are just a few examples to help you get the concept: `tanks.freshWater.currentLevel`, `performance.velocityMadeGood`, and `environment.outside.temperature`.

2. `pin` is the GPIO pin we'll be monitoring.

3. `read_delay` is the interval between reads of the pin, in ms.

The last two lines do the real work, using the three variables just defined:

4. `auto* pin_13_state = new DigitalInputState(pin, INPUT_PULLUP, read_delay);` - this line creates an instance of a DigitalInputState Sensor that's going to read GPIO13 every 500 ms. A [Sensor](../../concepts/#sensors) in SensESP is more than just a plain old physical sensor - it's a Class that knows how to read the value of a physical sensor, and it knows how often to read it. Those two things together make it able to function as an "event" in a ReactESP app, as explained above.

5. `pin_13_state->connect_to(new SKOutputBool(sk_path));` - this line passes along the value of `pin_13_state` to a special kind of [Transform](../../concepts/#transforms) called `SKOutput`, which has one job: send the data to Signal K. Since this is an `SKOutputBool`, it's expecting to *get* a Boolean in, and it *sends* a Boolean out. Its only parameter is the Signal K Path that will identify the data once it gets to Signal K.

The "passing along" of the value from `pin_13_state` to the `SKOutputBool` is done with a SensESP function called `connect_to()`. It passes the value of the object before it to the object after it. If you're interested in how it works at a low level, [click here](../../internals/#producers-and-consumers).

Note that the five lines above could have been written in only two lines, by skipping the first three lines. So *really*, the work done by this SensESP program is done with only two lines of code:

```c++
auto* pin_13_state = new DigitalInputState(13, INPUT_PULLUP, 500);
pin_13_state->connect_to(new SKOutputBool("digitalPin.13.state"));
```

## Summary

* A SensESP program typically functions like an event scheduler and executor, where the events are "read the data from a sensor every XXX ms and send it to Signal K".
* Most of the code in a typical SensESP program is boilerplate - it will be the same in every program.
* It takes very few lines of code to accomplish the main purpose of the program.

## Related Tutorials

* [Minimal Signal K with Two Sensors](../minimal_sk_two_sensors) shows how to read multiple physical sensors in a single SensESP project.
* [Using a SensESP-specific External Library](../one_wire).
* [Using a Generic Sensor's External Library](../bmp280).

[^1]: If you haven't set up your Signal K Server yet, you won't be able to tell if this Tutorial's program is working correctly, so [do that now](https://github.com/SignalK/signalk-server/blob/master/raspberry_pi_installation.md).

[^2]: If you haven't installed VS Code and PlatformIO yet, [do that now](https://platformio.org/install/ide?install=vscode).
