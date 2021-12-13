---
layout: default
title: Home
nav_order: 1
description: "SensESP is a universal sensor development framework for the ESP32 platform."
permalink: /
---
# Introduction

SensESP is a [Signal K](https://signalk.org) sensor development toolkit for the [ESP32](https://en.wikipedia.org/wiki/ESP32) platform.
If you are a boater (or a professional developer!) who wants to build a custom Wi-Fi connected sensor for your boat, this is the toolkit you need.
SensESP runs on commonly available ESP32 boards and helps you get sensor readings from physical sensors and transform them to meaningful data for Signal K or other outputs.

SensESP is built on the [Arduino](https://github.com/espressif/arduino-esp32) software development framework, a popular open source platform for embedded development.
(Note that this only refers to the software stack - Arduino hardware is not supported.)
To automate the management of external libraries, it also heavily relies on PlatformIO, a cross-platform build system for embedded devices (in other words, Arduino IDE is not supported).

SensESP features include:

- High-level programming interfaces for sensor development
- Supports for a wide range of common sensor hardware with a set of [add-on libraries](pages/additional_resources/add-ons/) - and if native support is missing, using existing Arduino libraries directly is also possible
- A Web configuration user interface for sensors, transforms, and output paths
- Easy on-boarding with a Wi-Fi configuration tool and fully automated server discovery
- Full Signal K integration with authentication, and transmission and reception of data
- Support for over-the-air (OTA) firmware updates
- Support for remote debugging over Wi-Fi

To use SensESP, you need an ESP32 development board and a way to power it from the boat's 12V or 24V nominal power system.
This can be done with commonly available ESP32DevKit boards and external DC-DC converters, or alternatively, the [Sailor Hat with ESP32 (SH-ESP32)](https://hatlabs.fi/product/sailor-hat-with-esp32/) has all these features baked into a developer-friendly board and enclosure kit.

Example use cases of SensESP include:

- Engine temperature measurement
- Switch input for bilge alarms
- Device control using relays
- Custom GPS and attitude sensors
- Engine RPM measurement
- Anchor chain counter
- Battery voltage and current measurement
- Tank level measurement
- Custom NMEA 2000 sensors

At heart, SensESP is a development toolkit, and not ready-made software.
This means that you will need to do some simple programming to use it.
Don't worry, though.
The project is newbie-friendly, so you don't need to know much to get started.
A lot of examples and tutorials are provided, and some related projects also provide ready-to-use firmware for specific use cases.

Some other projects such as [ESPHome](https://esphome.io/), [ESPEasy](https://github.com/letscontrolit/ESPEasy), and [Tasmota](https://tasmota.github.io/docs/) provide ready-to-use firmware for specific use cases.
Depending on your interests and skills, these projects may provide an easier way to get started, but if you want to properly integrate to the Signal K ecosystem, SensESP is the bee's knees!

Likewise, it is possible to create fully custom sensor firmware with the bare-metal Arduino Framework or esp-idf SDK, but that means having to implement all the nasty and boring parts such as the Signal K protocol, configuration management and so on yourself.

Interested in using SensESP? Get some hardware and follow our [Getting Started](pages/getting_started/) documentation!
