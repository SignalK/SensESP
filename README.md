# Signal K Outlook Guardian

The Outlook Guardian is a talking alarm annunciator for boats: an ESP32-S3 device that listens for vessel alarms and reads them out loud.

* Speaks the message of every [Signal K](https://signalk.org) notification that is raised
* Falls back to the NMEA 2000 alert messages (PGN 126983 and 126985) when the Signal K server is not connected
* Shows the severity of the alarm on the on-board RGB LED
* Two push buttons acknowledge and snooze the alarms, and tell the Signal K server so
* Six languages (US and UK English, Italian, German, Spanish, French) with a choice of voice, speed and volume in the web interface
* Speech is synthesized on the device with PicoTTS and played through a MAX98357A I2S amplifier; no internet connection is needed

It is built with SensESP, and this repository is a fork of [SignalK/SensESP](https://github.com/SignalK/SensESP) that adds the application and the drivers it needs.

## Hardware

* ESP32-S3-DevKitC-1 N16R8 (16 MB flash, 8 MB PSRAM)
* MAX98357A I2S amplifier and a speaker
* 3.3 V CAN transceiver for the NMEA 2000 bus
* Two push buttons

| Function | GPIO |
|----------|------|
| Amplifier LRC / BCLK / DIN | 4 / 5 / 6 |
| CAN TX / RX | 17 / 18 |
| Acknowledge button | 15 |
| Snooze button | 16 |
| RGB LED | 48 |

## Building and flashing

```sh
git clone --recursive https://github.com/OpenFairWind/S3-Outlook-Guardian.git
cd S3-Outlook-Guardian
pio run -e pioarduino_esp32s3 -t upload
```

The firmware is updated over USB. Over-the-air updates are not available, because the firmware with all six languages is too large to fit twice in the flash.

## Using it

On the first start the device opens a WiFi access point called `outlook-guardian` for entering the boat's WiFi settings.
Approve its access request on the Signal K server with read/write permission, then open `http://outlook-guardian.local/` and choose the language and voice under *Speech and alarms*.

| LED | Meaning |
|-----|---------|
| Off | No alarm |
| Blue | Alert or caution |
| Yellow | Warning |
| Red | Alarm |
| Red, blinking fast | Emergency |

The LED blinks until the alarms have been acknowledged or snoozed.

## Outlook Guardian documentation

The full documentation is in [`docs/pages/outlook_guardian`](docs/pages/outlook_guardian/index.md):

* [Hardware](docs/pages/outlook_guardian/hardware.md) - parts, wiring and pin assignments
* [Building and flashing](docs/pages/outlook_guardian/building.md)
* [Using the device](docs/pages/outlook_guardian/usage.md) - settings, LED and buttons
* [Alarm sources](docs/pages/outlook_guardian/alarm_sources.md) - how Signal K notifications and NMEA 2000 alerts are handled
* [Development notes](docs/pages/outlook_guardian/development.md) - code structure, languages and flash layout

## Status

The firmware builds, flashes and starts on the board. The audio output, the handling of live Signal K notifications and the reception of NMEA 2000 alerts have not yet been tested against real equipment.

# About SensESP

The rest of this file is the README of SensESP, the framework this project is built on.

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

> **Prefer an AI-assisted path?** The [SensESP Workspace](https://github.com/hatlabs/SensESP-workspace)
> is a [Claude Code](https://code.claude.com/docs/en/quickstart) environment that
> guides you from a natural-language description of what you want to build all the
> way to firmware flashed on your device — no prior embedded experience required.
> It supports Hat Labs boards (HALMET, HALSER, SH-ESP32, SH-wg) and generic ESP32s.

## Documentation

To get started, [see the project documentation](https://signalk.org/SensESP/).

For reference, the old SensESP version 1 documentation is still available at the [repository wiki](https://github.com/SignalK/SensESP/wiki).

## Getting help

Discussion about SensESP happens mostly on the [Signal K Discord](https://discord.com/channels/1170433917761892493/1170433918592368684) on the #sensesp channel. To register as a new user, follow the invite link at [signalk.org](https://signalk.org/).

Another place for SensESP discussions is the [discussion board on the SensESP GitHub repository](https://github.com/SignalK/SensESP/discussions).

## Version 3

SensESP v3 is the current major version. Key changes from v2 include:

- Web frontend rewritten with Preact and Bootstrap, replacing the old JSONEditor-based UI
- Built-in WiFi manager replacing the external WiFiManager library, with simultaneous AP+STA support
- Explicit `ConfigItem` system for exposing objects to the web configuration UI
- Transport-agnostic networking supporting both WiFi and Ethernet
- New transforms: Join, Zip, Filter, Throttle, Repeat variants, Hysteresis
- Standard ESP-IDF logging replacing RemoteDebug

For migration instructions from v2, see the [Migration Guide](https://signalk.org/SensESP/pages/migration/).

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
