## Change Log

Since new versions are not done regularly, this Change Log will list each PR as it is merged. It starts with
PR's since Version 0.4.3, since most of the work done prior to that was quite low-level, and not important
to most end users.

## Pull Requests Merged Since v0.4.3
- [#128](https://github.com/SignalK/SensESP/pull/128) (2020/05/13) feature: display IP address of ESP once connected to wifi
- [#127](https://github.com/SignalK/SensESP/pull/127) (2020/05/13) fix: small fixes to ESP32 support; update all examples for ESP32
- [#118](https://github.com/SignalK/SensESP/pull/118) (2020/05/25) feature: add support for ESP32
- [#125](https://github.com/SignalK/SensESP/pull/125) (2020/05/13) fix: DigitalInputValue was not getting to SK Server
- [#123](https://github.com/SignalK/SensESP/pull/123) (2020/05/10) feature: add SKValueListener, Threshold transform, DigitalOut sensor
- [#117](https://github.com/SignalK/SensESP/pull/117) (2020/04/30) fix: add Wire library to library.json
- [#113](https://github.com/SignalK/SensESP/pull/113) (2020/04/30) fix: add libraries for recent new sensors to library.json
- [#116](https://github.com/SignalK/SensESP/pull/116) (2020/04/28) fix: correct a comment in ina219.cpp
- [#112](https://github.com/SignalK/SensESP/pull/112) (2020/04/28) documentation: Clarify something in readme.md 
- [#110](https://github.com/SignalK/SensESP/pull/110) (2020/04/28) fix: add Adafruit INA219 library to platformio.ini
- [#108](https://github.com/SignalK/SensESP/pull/108) (2020/04/28) chore: give each ESP a unique SSID for its config access point
- [#107](https://github.com/SignalK/SensESP/pull/107) (2020/04/12) feature: add more options for including "Standard Sensors"
- [#104](https://github.com/SignalK/SensESP/pull/104) (2020/04/11) feature: add support for INA219 volt/amp sensor
- [#101](https://github.com/SignalK/SensESP/pull/101) (2020/04/06) feature: add support for SHT31 temp/humidity sensor
- [#100](https://github.com/SignalK/SensESP/pull/100) (2020/04/06) fix: remove "sensors" from all SK Paths throughout the project
- [#97](https://github.com/SignalK/SensESP/pull/97) (2020/04/06) feature: add support for BMP280 temp/pressure sensor
- [#96](https://github.com/SignalK/SensESP/pull/96) (2020/04/06) feature: add support for BME280 temp/pressure/humidity sensor
- [#82](https://github.com/SignalK/SensESP/pull/82) (2020/04/02) feature: add configurable read_delay to OneWire Temperature sensor
- [#81](https://github.com/SignalK/SensESP/pull/81) (2020/04/02) feature: add support for ADS1015/ADS1115 Analog-to-Digital Converters
- [#92](https://github.com/SignalK/SensESP/pull/92) (2020/04/01) fix: add "last value" to config of moving_average transport 
- [#84](https://github.com/SignalK/SensESP/pull/84) (2020/04/01) feature: update the example for AnalogInput
- [#83](https://github.com/SignalK/SensESP/pull/83) (2020/04/01) fix: allow max voltage to be set for AnalogVoltage sensor
- [#79](https://github.com/SignalK/SensESP/pull/79) (2020/04/01) feature: make load_configuration() error msgs specific to the cause
- [#78](https://github.com/SignalK/SensESP/pull/78) (2020/04/01) feature: ability to disable systemHz, freemen, uptime, and ipaddress
- [#77](https://github.com/SignalK/SensESP/pull/77) (2020/04/01) feature: add configurable read_delay to AnalogInput sensor
- [#91](https://github.com/SignalK/SensESP/pull/91) (2020/03/28) fix: add classname to change_filter() transform
- [#85](https://github.com/SignalK/SensESP/pull/85) (2020/03/23) fix: change "Sensor" to "Device" in config UI title
- [#80](https://github.com/SignalK/SensESP/pull/80) (2020/03/12) feature: add an example for OneWire Temperature sensors


### v0.4.3 (2019/10/28)
- [#69](https://github.com/SignalK/SensESP/pull/69) chore: change version to 0.4.3 and update Class documentation
