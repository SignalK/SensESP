---
layout: default
title: Minimal Signal K Example
parent: Tutorials
nav_order: 5
---

# Tutorial: Minimal Example of Sending Data to Signal K

By far the most common use case for SensESP is reading data from some kind of sensor and sending it to Signal K. This first tutorial will not only show how to do that, but will also explain every line in the program file (`main.cpp`) and some of the lines in the configuration file (`platformio.ini`). If you're new to SensESP, you are strongly encouraged to go through this Tutorial first, because all later Tutorials will refer to this one, and will *not* explain any of the boilerplate lines of code that appear in all SensESP programs.

This very basic example reads the state of GPIO pin #13 every 500 ms and sends the state to your Signal K Server[^1]. You can toggle the state of the pin by connecting a jumper wire to it and touching the other end to a GND pin, then releasing it. The pin's state (reported to Signal K as a Boolean) will toggle between TRUE and FALSE, and you can see that in the Signal K Data Browser.

## Downloading and Opening the Files

The files for this Tutorial are [in this GitHub repository](https://github.com/SensESP/Tutorial-Minimal-SignalK).(BAS: @mairas is going to rename this repo) You can look at them there if you like, but the steps below will download and open them with PlatformIO, which is where you'll want to be looking at them while going through the Tutorial.

1. Start Visual Studio Code[^2].
2. On the VS Code welcome page, click on "Clone Git Repository", and a little window will open at the top of the page for you to enter the URL of the repo.
3. Copy-paste this URL into that window, then click on "Clone from URL": `https://github.com/SensESP/Tutorial-Minimal-SignalK`. (BAS: this url will change.)
4. You'll be prompted for the folder where you want the repo to be cloned. Highlight the folder that will be the *parent* folder of this repo (such as `/Documents/MyGitHubProjects`), then click on the "Select Repository Location" button.
5. In the popup that appears in the lower right corner, click the "Open" button.
6. The Project will open, and `platformio.ini` will be open in the main Editor window. At the top of that window, you'll see a tab with "*platformio.ini*", in italics. Italics means the file is open, but as soon as you open another file, this one will close. So double-click on that tab, and the italics will change to normal text, indicating that the file will stay open when you open another file.
7. In the "Explorer" panel on the left, click on `> src` to open that folder, then double-click on `main.cpp` to open that file in the Editor. You now have open the only two files you'll need for this Tutorial.

## A Brief Overview of `platformio.ini`

Every PlatformIO project needs a `platformio.ini` file. The one you just downloaded was created to work with SensESP running on an ESP32. It contains the settings that PlatformIO needs to build this project and upload it to the ESP32. Feel free to browse through the file, but it's very unlikely that you'll ever have to change anything in it.

The one section that's important for this Tutorial is:

```c++
lib_deps =
   SignalK/SensESP @ ^2.0.0
```

`lib_deps` means "Library Dependencies", which must list every library that PlatformIO will need to build this project. Because this project uses only code that's in the SensESP core, the only library needed is `Signalk/SensESP @ ^2.0.0` (version 2.0.0 or higher).

Note that SensESP uses a lot of other libraries, but you don't list them here. That's because when PlatformIO downloads SensESP, it will automatically download all those other libraries, because the definition of SensESP tells it to.

## A Tour of `main.cpp`

In PlatformIO, the main program file that is compiled and built is always called `main.cpp`. The `main.cpp` for every SensESP project is going to have a lot of the same lines in it - the lines of "boilerplate" that make it a SensESP program. These are all described below.

### The Boilerplate

```c++
// BP:
#include <Arduino.h>
#include "sensesp_app.h"
#include "sensesp_app_builder.h"
#include "sensesp/signalk/signalk_output.h"

// Specific to this project
#include "sensesp/sensors/digital_input.h"
```

Every SensESP program that's sending data to SignalK will contain the first four `#include` statements above. After that, you will `#include` any other header files that you need for this particular project. In this case, we're using only one SensESP Sensor (one of the ones defined in `/sensesp/sensors/digital_input.h`), and no Transforms, so there is only one `#include` specific to this project. Other Tutorials will highlight the `#include` statements they need that are specific to them.

```c++
using namespace sensesp;
```

Every SensESP program will have this line. If you're not familiar with the "namespace" concept, think of it like this: a bunch of Classes and other things have been defined in some other files and identified collectively as "sensesp". This line of code tells the compiler where to find those things. Kind of like an `#include` statement, only different. But you don't need to understand it - it just needs to be in your program.

```c++
// BP: SensESP builds upon the ReactESP framework. Every ReactESP application
// must instantiate the "app" object.
reactesp::ReactESP app;
```

ReactESP is an application framework that was developed, in part, for SensESP. You can read all about it [here](https://github.com/mairas/ReactESP#reactesp) if you like, but you don't have to. Just understand that it functions like an event scheduler and executor. Within a ReactESP app, you define one or more events to occur, and then you start them. An example (explained in detail later) is what we're doing in this Tutorial: "read the state of GPIO13 every 500 ms and send it to Signal K". You can have one or many events defined in a single SensESP program.

```c++
// BP: The setup function performs one-time application initialization
void setup() {
```

SensESP's `main.cpp` is structured like an Arduino sketch, with `setup()` and `loop()` functions. This line is the beginning of the `setup()` function. Inside it, you define all of the ReactESP "events", and then start them at the end. All of the important stuff in a SensESP program happens either before, or inside of, `setup()`.

```c++
// BP: Some initialization boilerplate when in debug mode
#ifndef SERIAL_DEBUG_DISABLED
  SetupSerialDebug(115200);
#endif
```

SensESP, as well as many of the libraries it uses, can send messages to the PlatformIO Serial Monitor. These three lines are what enables that. If you want to *disable* it for some reason, simply add this to `main.cpp` anywhere above these lines: `#define SERIAL_DEBUG_DISABLED`.

```c++
  // BP: Create the global SensESPApp() object
  SensESPAppBuilder builder;
  sensesp_app = builder.get_app();
```

Every SensESP program has an instance of the C++ Class `SensESPApp`. It's what's known as a [singleton](https://refactoring.guru/design-patterns/singleton/cpp/example), so you don't use a typical constructor to create it. Instead, you create an instance of the Class `SensESPAppBuilder`, which creates the SensESPApp object for you.

The two lines above are in every SensESP program, but sometimes there will be one or more other lines in between them, used to hard-code some program attributes. (If you're curious, [click here](../../getting_started/#hard-coding-certain-program-attributes) to see what those lines might be. But you can wait - there's another Tutorial that will explain it in detail.)

After the `sensesp_app = builder.get_app();` line comes the real meat of the program - setting up the "event(s)" that the program will execute over and over. Those are the lines that make each program different, and they'll be described in detail for each of the Tutorials. But there's one last section of boilerplate to describe:

```c++
  // BP: Start the SensESP application running
  sensesp_app->start();
}

// BP: loop() simply calls `app.tick()` which will then execute all reactions as needed
void loop() {  
  app.tick(); 
}

```

The lines above should be at the end of every SensESP program. The first one, `sensesp_app->start();`, tells the ReactESP framework that all events have been defined, so it's time to start them all. The next line after that - the right curly bracket - is simply the end of the `setup()` funtion.

The last three lines define the `loop()` function, which, in the ReactESP framework, looks for an "event" to execute at every tick of the microprocessor's clock.

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

4. `auto* pin_13_state = new DigitalInputState(pin, INPUT_PULLUP, read_delay);` - this line creates a pointer (called `pin_13_state`) to a DigitalInputState Sensor that's going to read GPIO13 every 500 ms. A [Sensor](../../concepts/#sensors) in SensESP is more than just a plain old physical sensor - it knows how to read the value of a physical sensor, and it knows how often to read it. Those two things together make it able to function as an "event" in a ReactESP app, as explained above in the [Boilerplate section](#the-boilerplate).

5. `pin_13_state->connect_to(new SKOutputBool(sk_path));` - this line takes the value of `pin_13_state` (every 500 ms, as defined above) and passes it along to a special kind of [Transform](../../concepts/#transforms) called `SKOutput`, which has one job: send the data to Signal K. Since this is an `SKOutputBool`, it's expecting to *get* a Boolean in, and it *sends* a Boolean out. Its only parameter (in this case) is the Signal K Path that will identify the data to Signal K. 

The "passing along" of the value from `pin_13_state` to the `SKOutputBool` is done with a SensESP function called `connect_to()`. It passes the value of the object before it to the object after it. If you're interested in how it works at a low level, [click here](../../internals/#producers-and-consumers).

Note that the five lines above could have been written in only two lines, by skipping the first three lines. So *really*, the work done by this SensESP program is done with only two lines of code:

```c++
auto* pin_13_state = new DigitalInputState(13, INPUT_PULLUP, 500);`
pin_13_state->connect_to(new SKOutputBool("digitalPin.13.state"));
```

## Summary

* A SensESP program typically functions like an event scheduler and executor, where the events are "read the data from a sensor every XXX ms and send it to Signal K".
* Most of the code in a typical SensESP program is boilerplate - it will be the same in every program.
* It takes very few lines of code (really only two in this Tutorial) to accomplish the purpose of the program.

## Related Tutorials

* [Minimal Signal K with Two Sensors](../minimal_sk_two_sensors) shows how to read multiple physical sensors in a single SensESP project.
* [Using a SensESP-specific External Library](../one_wire).
* [Using a Generic Sensor's External Library](../bmp280).

[^1]: If you haven't set up your Signal K Server yet, you won't be able to tell if this Tutorial's program is working correctly, so [do that now](https://github.com/SignalK/signalk-server/blob/master/raspberry_pi_installation.md).

[^2]: If you haven't installed VS Code and PlatformIO yet, [do that now](https://platformio.org/install/ide?install=vscode).
