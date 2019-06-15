# SensESP

SensESP is a Signal K sensor development library for ESP8266
and ESP32. It can be used as a high-level toolkit for
creating ESP hardware sensoring devices that interface with Signal K
servers. At the moment it is very much a work in progress.

SensESP is heavily inspired by [SigkSens](https://github.com/mxtommy/SigkSens)
and prior work done by [@mxtommy](https://github.com/mxtommy).

Class documentation for SensESP is [here](http://signalk.org/SensESP/annotated.html)

## Getting Started

You must have a Signal K Server running on your network, or SensESP has nothing to connect to. The most common installation is the
Signal K node server running on a Raspberry Pi. Installation instructions for that are [here](https://github.com/SignalK/signalk-server-node/blob/master/raspberry_pi_installation.md).

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
select "New Project". Enter a name (something like SensESPTest for your first project) and then select "WeMos
D1 mini" in the board dropdown. The Arduino framework
should become automatically selected. Complete the new
project dialog, then open the project you created.

Once you have your new project open, open the `platformio.ini` file that's in your project's directory (NOT the one that you find if you go down into the .pio/libdeps/... folders). Add the SensESP dependency to the section for your Wemos (the section title should be `[env:d1_mini]`):
```
lib_deps =
    https://github.com/SignalK/SensESP.git
```
Then, open `src/main.cpp`. The default template is for Arduino IDE, but a SensESP main file will look very different. Replace the `main.cpp` contents with one of the SensESP examples in the `examples` subdirectory. Check that the settings match your hardware and select "Build" from the PlatformIO submenu (the little alien face) from the left toolbar. If the build succeeds, you can plug in your Wemos board and press "Upload and Monitor".

Assuming the project uploading was successful, your Wemos will be running the example code. To configure it, connect your computer or phone wifi to the "Unconfigured Sensor" network. A captive portal may pop up, but if it doesn't, open a browser and go to 192.168.4.1. Enter your wifi credentials to allow the device to access the network. Also enter a suitable name, for example `WemosSensESP` for the device. (No more than 16 characters, no spaces.) Save the configuration with the button on the bottom of the page, and the Wemos will restart and try to connect to your wifi network.

Once on the network, SensESP should automatically find your Signal K server, assuming it has mDNS enabled. If your server has security enabled, you should
see an access request for Wemos in the Signal K Dashboard, under Security - Access Requests. (You must be logged into the Signal K Dashboard to see the Security sub-menu.) Approve it, choosing "Never" for the expiration. The Wemos will restart, and you should start getting data on the Signal K Instrument Panel. (Dashboard - Webapps - Instrument Panel)

## Low-level wiring

TODO!

## Runtime setup

In the examples below, `sensesp` is used as the device name.

You can configure your device with any web browser by going to

    http://sensesp.local


SensESP also implements a RESTful configuration API. A list of
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
- [x] Web configuration UI
- [ ] Control device support. For now, all devices are read-only, and controls such as leds, relays, or
PWM output are not supported.

SensESP Class Diagram
----------------------------
![alt text](sens_esp_uml.png "UML for SensESP")
