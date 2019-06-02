# SensESP

SensESP is a Signal K sensor development library for ESP8266
and ESP32. It can be used as a high-level toolkit for
creating ESP sensor devices that interface with Signal K
servers. At the moment it is very much a work in progress.

SensESP is heavily inspired by [SigkSens](https://github.com/mxtommy/SigkSens)
and prior work done by [@mxtommy](https://github.com/mxtommy).

Class documentation for SensESP is [here](http://signalk.org/SensESP/annotated.html)

## Getting Started

SensESP is a library and will be installed automatically as
a dependency when defined as such in the project's
`platformio.ini` file.

You need to have PlatformIO installed. First, download and
install [Visual Studio Code](https://code.visualstudio.com/).
Then, select "Extensions" from the left toolbar
(see image XX). Search for "platformio", select the first
result, and press "Install".

Once you have PlatformIO installed, its home screen should
open automatically when you start VSCode. On the PIO Home,
select "New Project". Enter a name and then select "WeMos
D1 mini" in the board dropdown. The Arduino framework
should become automatically selected. Complete the new
project dialog and open the project you created.

Once you have your new project open, open the `platformio.ini` file. Add the SensESP dependency. Eventually, when SensESP is fully registered with PlatformIO, you'll be able to use:

```
lib_deps =
    SensESP
```
But for now, use this:
```
https://github.com/SignalK/SensESP.git
```

Then, open `src/main.cpp`. The default template is for Arduino IDE and the SensESP main file will look a bit different. Replace the `main.cpp` contents with one of the SensESP examples in the `examples` subdirectory. Check that the settings match your hardware and select "Build" from the PlatformIO submenu (the little alien face) from the left  toolbar. If the build succeeds, you can plug in your Wemos board and press "Upload and Monitor".

Assuming the project uploading was successful, you will have a sensor running the sample code. To configure it, connect your computer or phone wifi to the "Unconfigured Sensor" network. A captive portal should pop up. Enter your wifi credentials to allow the device to access the network. Also enter a suitable name, for example `sensesp` for the device.

Once on the network, your new SensESP device should automatically find your Signal K server, assuming it has mDNS enabled. If your server has security enabled, you should
get an access request for the device. Approve it, preferably with a long or infinite duration, and you should start getting data on the instrument panel.

## Low-level wiring

TODO!

## Runtime setup

In the examples below, `sensesp` is used as the device name.

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
- [x] Make the project a library
- [ ] Web configuration UI
- [ ] Control device support. For now, all devices are read-only, and control devices such as leds, relays, or
PWM output are not supported.

SensESP Class Diagram
----------------------------
![alt text](sens_esp_uml.png "UML for SensESP")
