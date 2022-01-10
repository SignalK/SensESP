---
layout: default
title: Using a SensESP-specific External Library - 1-Wire Temperature
parent: Tutorials
nav_order: 15
---

# Tutorial: Reading a Sensor with a SensESP-specific External Library: 1-Wire Temperature

Although SensESP can read virtually any Arduino-compatible sensor, there are only two types of sensor that SensESP can read without using at least one external library: those that can be read with AnalogRead() or DigitalRead(). Every other kind of sensor - for example, those that use I2C or SPI - require at least one external library to work with SensESP.

Most sensors that use an external library can easily be used with SensESP with just a few lines of code, as illustrated in [this Tutorial](../bmp280) showing how to read a BMP280 temperature and pressure sensor. But some are not so simple, so we've created some "helper" libraries that make it easier to use them with SensESP. You can find these [here](https://github.com/SensESP). As of this writing, these special "helper" libraries include:

* [1-Wire Temperature Sensors](https://github.com/SensESP/OneWire)
* [MAX31856 Thermocouple Sensors](https://github.com/SensESP/MAX31856)
* [Victron Energy MPPT Devices](https://github.com/SensESP/ve.direct_mppt)

BAS: this will use the existing SensESP/OneWire example that @mairas recently rewrote. Need to explain how to use it (copy the example main.cpp into YOUR main.cpp, and modify your platformio.ini - right?).