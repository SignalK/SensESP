# SensESP

SensESP is a Signal K sensor framework for ESP8266 and ESP32. At the moment it is very much a work in progress.

SensESP is heavily inspired by [SigkSens](https://github.com/mxtommy/SigkSens) and prior work done by [@mxtommy](https://github.com/mxtommy).

## Installation

SensESP needs to be installed using PlatformIO:

1. Clone or download the GitHub repo.
2. Connect a Wemos (or similar) ESP8266 board to your computer.
3. Open the directory in PlatformIO.
4. Press the "Upload and Monitor" button.
5. See the magic happen.

## Initial device wiring

Initial device wiring happens in `main.cpp` using either the
high-level wiring helpers defined in `wiring_helpers.h`, or
the low-level devices and transforms defined in `devices/*`
and `transforms/*`, respectively.

## Runtime setup

Once the software has been uploaded, connect to the
"Unconfigured sensor" Wifi access point. A captive portal
should automatically appear. Select your wifi and enter
the credentials. Also enter a suitable hostname for the
device. In the examples below, `sensesp` is used.

SensESP implements a RESTful configuration API. A list of
possible configuration keys can be retrieved from:

    http://sensesp.local/config

An individual key configuration can be retrieved under
the config path:

    http://sensesp.local/config/system/networking

Configuration can be updated with HTTP PUT requests:

    curl -X PUT \
    http://sensesp.local/config/system/networking \
    -H 'Content-Type: application/json' \
    -d '{
      "hostname": "myesp"
    }'

## Feature wishlist

- [x] Persistent configuration
- [x] RESTful HTTP control interface
- [x] Improved device configuration system
- [x] Authentication token support
- [ ] Make the project a library
- [ ] Web configuration UI
- [ ] Control device support. For now, all devices are read-only, and control devices such as leds, relays, or
PWM output are not supported.
