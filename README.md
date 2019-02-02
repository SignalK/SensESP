# SensESP

SensESP is a Signal K sensor framework for ESP8266 and ESP32. At the moment it should be considered a proof of concept, heavily inspired by [SigkSens](https://github.com/mxtommy/SigkSens).

## Installation

SensESP needs to be installed using PlatformIO:

1. Clone or download the GitHub repo.
2. Connect a Wemos (or similar) ESP8266 board to your computer.
3. Open the directory in PlatformIO.
4. Press the "Upload and Monitor" button.
5. See the magic happen.

## Initial device configuration

Initial device configuration is at the moment performed by
modifying the `SensESPApp` constructor in `app.cpp`.
(Sorry about that.)

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

    http://sensesp.local/config/comp/uptime

Configuration can be updated with HTTP PUT requests:

    curl -X PUT \
    http://sensesp.local/config/comp/uptime \
    -H 'Content-Type: application/json' \
    -d '{
      "k": 1,
      "c": 0
    }'

## Feature wishlist

- [x] Persistent configuration
- [x] RESTful HTTP control interface
- [ ] Improved device configuration system
- [ ] Authentication token support
- [ ] Web configuration UI
- [ ] Device writing support