---
layout: default
title: Additional Resources
nav_order: 120
has_children: true
---

# Additional Resources

## Class documentation

Class documentation for SensESP is [here](../../generated/docs/annotated.html).

## List of SensESP Add-ons and Related Projects

This page lists different add-ons and other projects related to SensESP.

If you want to have your own project added to the list, [create an issue](https://github.com/SignalK/SensESP/issues) requesting that it be added, or [make a PR](https://github.com/SignalK/SensESP/pulls) to modify this page (`/docs/pages/additional_resources/index.md`).

## Sensor Libraries

### Attitude

- [NXP Sensor Fusion](https://github.com/BjarneBitscrambler/OrientationSensorFusion-ESP): Orientation sensing and sensor fusion using NXP FXOS8700 magnetometer/accelerometer and FXAS21002 gyroscope sensors
- [SignalK-Orientation](https://github.com/BjarneBitscrambler/SignalK-Orientation): Library providing vessel and vehicle orientation information in Signal K message format

### Current and Power

- [INA2xx](https://github.com/SensESP/INA2xx): Library for reading one or more of the INA2xx (INA219, INA226, INA3221) current and power sensors
- [ve.direct_mppt](https://github.com/SensESP/ve.direct_mppt): Library for reading one victron mppt unit via the ve.direct hardware interface into SignalK/SensESP

### Control and Display Libraries

- [Status display for SSD1306-based OLED modules](https://github.com/mairas/SensESPStatusDisplay): An example for using small 128x32 and 128x64 pixel OLED modules as a system status display
- [Electronic "analog" gauge](https://github.com/joelkoz/signalk-analog-gauge): Replace an old analog gauge with an electronic version of one, that also sends its data to Signal K
- [SmartSwitch Controller](https://github.com/joelkoz/CommonSens/tree/main/signalk-smart-switch): Library to use various SmartSwitch devices (from Sonoff, Shelly, and M5) with Signal K

### Temperature and Humidity

- [Adafruit MAX31856 thermocouple sensor shield](https://github.com/SensESP/MAX31856): Library for reading a Type K thermocouple with an Adafruit MAX31856 breakout board
- [1-Wire temperature sensors](https://github.com/SensESP/OneWire): Library for reading multiple 1-Wire temperature sensors, such as the DS18B20
- [SH-ESP32 temperature sensing tutorial](https://hatlabs.github.io/sh-esp32/pages/tutorials/onewire-temperature/): A step-by-step tutorial for building an NMEA 2000-enabled SensESP temperature sensor device with the SH-ESP32 development board

## Examples and Related Projects

- [SH-ESP32-test-jig](https://github.com/hatlabs/SH-ESP32-test-jig): A test jig implementation for the [SH-ESP32](https://hatlabs.github.io/sh-esp32/). While not generally useful in itself, these two projects showcase NMEA 2000 communication and how to use SensESP constructs without having to instantiate a SensESPApp object.
