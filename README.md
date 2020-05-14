# SensESP

SensESP is a Signal K sensor development library for ESP8266
and ESP32. It can be used as a high-level toolkit for
creating ESP hardware sensoring devices that interface with Signal K
servers. At the moment it is very much a work in progress.

SensESP is heavily inspired by [SigkSens](https://github.com/mxtommy/SigkSens)
and prior work done by [@mxtommy](https://github.com/mxtommy).

Class documentation for SensESP is [here](http://signalk.org/SensESP/annotated.html).

A Wiki page with more detailed information about using SensESP is [here](https://github.com/SignalK/SensESP/wiki).

## Getting Started

You must have a Signal K Server running on your network, or SensESP has nothing to connect to. The most common installation is the Signal K node server running on a Raspberry Pi. Installation instructions for that are [here](https://github.com/SignalK/signalk-server-node/blob/master/raspberry_pi_installation.md).

Once the SK Server is installed and running, go to the Dashboard (enter `localhost:3000` into the Raspberry Pi's browser to start it), select Server - Settings from the left side menu, and make sure the "mdns" option is ON.

SensESP is a library and will be installed automatically as a dependency when defined as such in the project's
`platformio.ini` file. Instructions below.

You need to have PlatformIO installed. First, download and install [Visual Studio Code](https://code.visualstudio.com/).
Then, select "Extensions" from the left toolbar. Search for "platformio", select the first result, and press "Install".

Once you have PlatformIO installed, its home screen should open automatically when you start VSCode. On the PIO Home,
select "New Project". Enter a name (something like SensESPTest for your first project) and then select "WeMos
D1 R2 and mini" in the board dropdown. (This assumes you're using the most commonly used microcontroller with SensESP,
the Wemos D1 mini, an implementation of the ESP8266 chip. If you're using a different ESP, select that in the 
board dropdown.) The Arduino framework should become automatically selected. Complete the New Project dialog, then open the project you created.

Once you have your new project open, open the `platformio.ini` file that's in your project's directory (NOT the one that you find if you go down into the .pio/libdeps/... folders). Add the SensESP dependency to the section for your Wemos (the section title should be `[env:d1_mini]`):
```
lib_deps =
    https://github.com/SignalK/SensESP.git
```
Then, open `src/main.cpp`. The default template is for the Arduino IDE, but a SensESP main file will look very different. Replace the contents of `main.cpp` with the contents of one of the SensESP examples in the `examples` subdirectory here on GitHub. (This is a good one to start with: https://github.com/SignalK/SensESP/blob/master/examples/analog_input.cpp .) Check that the settings match your hardware and select "Build" from the PlatformIO submenu (the little alien face) on the left toolbar. If the build succeeds, you can plug in your ESP board and press "Upload and Monitor".

If you see an error about a missing file or directory, similar to this:
`.pio/libdeps/d1_mini/Adafruit BusIO_ID6214/Adafruit_I2CDevice.h:1:18: fatal error: Wire.h: No such file or directory`
it's probably not finding all the necessary libraries. Look at `https://github.com/SignalK/SensESP/blob/master/platformio.ini`, in the `lib_deps` section. You may need to copy one or more of these libraries into the `lib_deps` section of the `platformio.ini` file in your Project folder:
```
    Adafruit ADS1X15
    Wire
    Adafruit BME280 Library
    etc.
```

If the project compiles and uploads, your ESP will be running the example code. Since the first thing it needs to do is connect to a wifi network, and it doesn't know what network to connect to, it will broadcast a wifi SSID for you to connect to so you can configure it. Connect your computer or phone wifi to the "Configure sensesp" network. A captive portal may pop up, but if it doesn't, open a browser and go to 192.168.4.1. Enter your wifi credentials to allow the device to access the network that your Signal K Server is on. Also enter a suitable name, for example `BilgeMonitor` for the device. (No more than 16 characters, no spaces.) Save the configuration with the button on the bottom of the page, and the ESP will restart and try to connect to your wifi network.

Once on the network, SensESP should automatically find your Signal K server, assuming it has mDNS enabled (see instructions above). If your server has security enabled (it does by default), you should see an access request for yuor ESP in the Signal K Dashboard, under Security - Access Requests. (You must be logged into the Signal K Dashboard to see the Security sub-menu.) Leave the "Authentication Timeout" field empty, set the Permission to "Read / Write", then Approve it. The ESP will restart, and you should start getting data on the Signal K Instrument Panel. (Dashboard - Webapps - Instrument Panel)

## Low-level wiring

TODO!

## Runtime setup

In the examples below, `sensesp` is used as the device name.

You can configure your device with any web browser by putting the IP address of your device into the URL field of the browser, or by going to

    http://sensesp.local


Everything that is configurable on a "live" device will be in the menu that appears. You can also restart the device from that menu, and even reset the device to factory settings.

(OBSOLETE?) SensESP also implements a RESTful configuration API. A list of
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
- [x] Control device support. Support for SKValueListener is working with WS subscription. Relay sample is working, LEDs should work.

SensESP Class Diagram
----------------------------
![alt text](sens_esp_uml.png "UML for SensESP")
