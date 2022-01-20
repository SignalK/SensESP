# SensESP

## Note About the Repository Branches

As part of the Version 2.0.0 release, version 1 content has been moved to the [v1](https://github.com/SignalK/SensESP/tree/v1) branch.
The long-lived `v2-dev` feature branch has been renamed to `main` and is now the default branch.
To highlight the change, the `master` branch is now gone.

To update your local repository to reflect the branch renames, run the following commands:

```bash
git branch -m master main
git fetch origin
git branch -u origin/main main
git remote set-head origin -a
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
* Support for a wide range of common sensor hardware with a set of [add-on libraries](pages/additional_resources/add-ons/) - and if native support is missing, using existing Arduino libraries directly is also quite simple in most cases
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

Discussion about SensESP happens mostly in [Signalk-dev Slack](http://slack-invite.signalk.org/) on the #sensors channel. Don't hesitate to join and ask if you ever have problems with getting your project working!
