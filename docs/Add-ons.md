# List of SensESP add-ons and related projects

This page lists different add-ons and other projects related to [SensESP](https://github.com/SignalK/SensESP).

If you want to have your own project added to the list, [make a PR](https://github.com/SignalK/SensESP/pulls) or [create an issue](https://github.com/SignalK/SensESP/issues) for it.

## Sensor libraries

### Attitude

- [NXP Sensor Fusion](https://github.com/BjarneBitscrambler/OrientationSensorFusion-ESP): Orientation sensing and sensor fusion using NXP FXOS8700 magnetometer/accelerometer and FXAS21002 gyroscope sensors

### Current and power

- [INA2xx](https://github.com/SensESP/INA2xx): Library for reading one or more of the INA2xx (INA219, INA226, INA3221) current and power sensors
- [ve.direct_mppt](https://github.com/SensESP/ve.direct_mppt): Library for reading one victron mppt unit via the ve.direct hardware interface into SignalK/SensESP

### Output and display libraries

- [Status display for SSD1306-based OLED modules](https://github.com/mairas/SensESPStatusDisplay): An example for using small 128x32 and 128x64 pixel OLED modules as a system status display

### Temperature and humidity

- [DHTxx](https://github.com/SensESP/DHTxx): Library for reading low-cost DHTxx Temperature and Humidity Sensors including DHT11, DHT21, and DHT22.
- [SH-ESP32 temperature sensing tutorial](https://hatlabs.github.io/sh-esp32/pages/tutorials/onewire-temperature/): A step-by-step tutorial for building an NMEA 2000 enabled SensESP temperature sensor device with the SH-ESP32 development board

## Examples and related projects

- [SH-ESP32-test-jig](https://github.com/hatlabs/SH-ESP32-test-jig): A test jig implementation for the [SH-ESP32](https://hatlabs.github.io/sh-esp32/). While not generally useful in itself, these two projects showcase NMEA 2000 communication and how to use SensESP constructs without having to instantiate a SensESPApp object.
