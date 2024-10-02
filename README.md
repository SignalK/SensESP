# SensESP

## Getting Started with Version 3

### Major Changes

- WiFiManager replaced with built-in implementation
  - Supports simultaneous client and AP
  - Same frontend used as captive portal
  - Support for multiple alternative WiFi networks
- Web server replaced with ESP-IDF standard implementation
- Websockets replaced with ESP-IDF standard implementation
- AsyncTCP dependencies removed
- RemoteDebug replaced with ESP-IDF debugging macros
- Web Frontend rewritten from scratch with Preact and Bootstrap
  - JSONEditor no longer used
  - More flexible and user-friendly UI
  - Support for authentication
  - Support for frontend plugins (app-defined React pages)
  - Indicate when a ocnfiguration change requires a restart

- Remove Startable class
- Rename classes:
  - `SensorT` -> `Sensor`
  - `IntegratorT` -> `Integrator`
  - `DebounceTemplate` -> `Debounce`
  - `WSClient` -> `SKWSClient`
  -
- Add a run script to help with SensESP development
- Function signature changes:
  - Pass received JSON objects
  - Rename `ValueConsumer::set_input` to `ValueConsumer::set`. The value parameter
    is now passed as a const reference. The input_channel parameter is removed.

- Add new transforms:
  - `ExpiringValue`: output value expires after a given time unless updated
  - `Repeat`: repeats the input value at a given interval
  - `RepeatStopping`: repeats the input value at a given interval, stopping after a given time
  - `RepeatExpiring`: repeats the input value at a given interval, expiring after a given time.
    `RepeatExpiring` outputs an `Nullable<T>` value, where the value is invalid/`null` after expiration.
  - `RepeatConstantRate`: repeats the input value at a constant rate, regardless of input rate.
    `RepeatConstantRate` outputs an `Nullable<T>` value, where the value is invalid/`null` after expiration.
  - `Join`: joins multiple input values into a single output tuple that is emitted when any inputs are updated
  - `Zip`: joins multiple input values into a single output tuple that is emitted when all inputs are updated
  - `Throttle`: limits the rate of output updates
  - `Filter`: emits the input value only if it passes a given test

- Support asynchronous configurables for setting remote device configuration
  using the web interface

- Implement stream producers that emit characters or lines from a stream
  (e.g., a serial port)
- SKEmitter::as_signalk() has been renamed to SKEmitter::as_signalk_json().
  It now writes its output to a JsonDocument reference instead
  of returning a String.

- Custom Signal K output is done differently than before. Instead of implementing
  an `as_signalk()` method in your `SKOutput` specialization that returned a
  serialized string, you should
  implement an `as_signalk_json()` method that writes constructs an ArduinoJson
  object in the provided `JsonObject` reference.

### Migrating Existing Projects

- Update your project's `platformio.ini` file to use the new version of SensESP:

  ```ini
  lib_deps =
    SignalK/SensESP @ >=3.0.0-beta.2,<4
    # https://github.com/SignalK/SensESP.git  # Use this line to use the latest git version
    # symlink:///Users/mairas/src/SignalK/SensESP  # Use this line to use a local copy
  ```

- Adjust the build flags in your project's `platformio.ini` file as follows:

  ```ini
  build_flags =
    -D LED_BUILTIN=2
    ; Max (and default) debugging level in Arduino ESP32 Core
    -D CORE_DEBUG_LEVEL=ARDUHAL_LOG_LEVEL_VERBOSE
    ; Arduino Core bug workaround: define the log tag for the Arduino
    ; logging macros.
    -D TAG='"Arduino"'
    ; Use the ESP-IDF logging library - required by SensESP.
    -D USE_ESP_IDF_LOG
  ```

- If you have the following in the beginning of your `setup()` function:

  ```cpp
  #ifndef SERIAL_DEBUG_DISABLED
    SetupSerialDebug(115200);
  #endif
  ```
  replace it with:

  ```cpp
  SetupLogging();
  ```

- Remove the final `sensesp_app->start();` call from your `setup()` function.

- If your project depends on any external libraries, they may need to be updated
  to work with the new version of SensESP.

- The `reactesp` namespace is no longer imported. If you have any references to
  classes in this namespace, you will need to update them to use the namespace
  explicitly.

  Additionally, ReactESP classes have been renamed:

  - `ReactESP` -> `reactesp::EventLoop`
  - `*Reaction` -> `reactesp::*Event`

  For example, `ReactESP` class should be referred to as
  `reactesp::EventLoop`. In particular, this change probably needs to be made
  in your project's `main.cpp` file.

- `ReactESP` is no longer a singleton. Earlier, you could refer to the singleton
  instance of `ReactESP` using `ReactESP::app`. Now, the object pointer is
  maintained by the `SensESPBaseApp` class, and you can refer to it using
  `sensesp_app->get_event_loop()`. For example:

  ```cpp
  event_loop()->onRepeat(
    1000,
    []() { Serial.println("Hello, world!"); }
  );
  ```

### Development

The frontend project is in the `frontend` directory tree. To build the frontend,
you need to have Node and `pnpm` installed. Install the required dependencies:

```sh
./run install-frontend
```

Then build the frontend:

```sh
./run build-frontend
```

This will both build the project and save the deployment files in C++ source files suitable for embedding in the SensESP firmware.

To start the development server, run:

```sh
cd frontend
pnpm run dev
```

## Introduction

**TL;DR: [Click here for the Getting Started documentation!](https://signalk.org/SensESP/pages/getting_started/)**

SensESP is a [Signal K](https://signalk.org) sensor development toolkit for the [ESP32](https://en.wikipedia.org/wiki/ESP32) platform.
If you are a boater (or a professional developer!) who wants to build a custom Wi-Fi connected sensor for your boat, this is the toolkit you need.
SensESP runs on commonly available ESP32 boards and helps you get sensor readings from physical sensors and transform them to meaningful data for Signal K or other outputs.

SensESP is built on the [Arduino](https://github.com/espressif/arduino-esp32) software development framework, a popular open source platform for embedded development.
(Note that this refers only to the software stack - Arduino hardware is not supported.)
To automate the management of external libraries, it also heavily relies on PlatformIO, a cross-platform build system for embedded devices (in other words, Arduino IDE is not supported).

SensESP features include:

* High-level programming interfaces for sensor development
* Support for a wide range of common sensor hardware with a set of [add-on libraries](https://signalk.org/SensESP/pages/additional_resources/) - and if native support is missing, using existing Arduino libraries directly is also quite simple in most cases
* A Web configuration user interface for sensors, transforms, and output paths
* Easy on-boarding with a Wi-Fi configuration tool and fully automated server discovery
* Full Signal K integration with authentication, and transmission and reception of data
* Support for over-the-air (OTA) firmware updates
* Support for remote debugging over Wi-Fi

To use SensESP, you need an ESP32 development board and a way to power it from the boat's 12V or 24V nominal power system.
This can be done with commonly available ESP32DevKit boards and external DC-DC converters, or alternatively, the [Sailor Hat with ESP32 (SH-ESP32)](https://hatlabs.fi/product/sailor-hat-with-esp32/) has all these features baked into a developer-friendly board and enclosure kit.

Example use cases of SensESP include:

* Engine temperature measurement
* Switch input for bilge alarms
* Device control using relays
* Custom GPS and attitude sensors
* Engine RPM measurement
* Anchor chain counter
* Battery voltage and current measurement
* Tank level measurement
* Custom NMEA 2000 sensors

At heart, SensESP is a development toolkit, and not ready-made software.
This means that you will need to do some simple programming to use it.
Don't worry, though.
The project is newbie-friendly, so you don't need to know much to get started.
A lot of examples and tutorials are provided, and some related projects also provide ready-to-use firmware for specific use cases.

## Documentation

To get started, [see the project documentation](https://signalk.org/SensESP/).

For reference, the old SensESP version 1 documentation is still available at the [repository wiki](https://github.com/SignalK/SensESP/wiki).

## Getting help

Discussion about SensESP happens mostly on the [Signal K Discord](https://discord.com/channels/1170433917761892493/1170433918592368684) on the #sensesp channel. To register as a new user, follow the invite link at [signalk.org](https://signalk.org/).

Another place for SensESP discussions is the [discussion board on the SensESP GitHub repository](https://github.com/SignalK/SensESP/discussions).
