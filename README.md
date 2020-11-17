# SensESP

SensESP is a Signal K sensor development library for ESP8266
and ESP32. It can be used as a high-level toolkit for
creating ESP-based hardware sensoring devices that interface with Signal K
servers.

SensESP is heavily inspired by [SigkSens](https://github.com/mxtommy/SigkSens)
and prior work done by [@mxtommy](https://github.com/mxtommy).

Class documentation for SensESP is [here](http://signalk.org/SensESP/generated/docs/annotated.html).

A Wiki page with more detailed information about using SensESP is [here](https://github.com/SignalK/SensESP/wiki).

*NOTE:* If you have an existing project that fails to compile after 2020-10-22, add the following line to the `[env]` section of the `platformio.ini` file of your project:

    lib_ldf_mode = deep

## Getting Started

You must have a Signal K Server running on your network, or SensESP has nothing to connect to. The most common installation is the Signal K node server running on a Raspberry Pi. Installation instructions for that are [here](https://github.com/SignalK/signalk-server-node/blob/master/raspberry_pi_installation.md).

Once the SK Server is installed and running, go to the Dashboard (enter `localhost:3000` into the Raspberry Pi's browser to start it), select Server - Settings from the left side menu, and make sure the "mdns" option is ON.

SensESP is a library and will be installed automatically as a dependency when defined as such in the project's
`platformio.ini` file. Instructions below.

You need to have PlatformIO installed. First, download and install [Visual Studio Code](https://code.visualstudio.com/).
Then, select "Extensions" from the left toolbar. Search for "platformio", select the first result, and click "Install".

Once you have PlatformIO installed, its home screen should open automatically when you start VSCode. On the PIO Home, 
select "New Project". Enter a name (something like SensESPTest for your first project) and then select "WeMos
D1 R2 and mini" in the board dropdown. (This assumes you're using the most commonly used microcontroller with SensESP, 
the Wemos D1 mini, an implementation of the ESP8266 chip. If you're using a different ESP, select that in the 
board dropdown. If you're using an ESP32 that isn't in the board list, choose "Espressif ESP32 Dev Module".)
The Arduino framework should become automatically selected. Complete the New Project dialog, then open the project you created.

Once you have your new project open, open the automatically generated `platformio.ini` file that's in your project's directory (NOT the one that you find if you go down into the .pio/libdeps/... folders). Save it as `platformio.ini.auto` so you can refer back to it.

Create a new file, and copy/paste the entire contents of [this example `platformio.ini` file](https://github.com/SignalK/SensESP/blob/master/examples/platformio.ini) into it. Save it as `platformio.ini`.

If the `[platformio]` section of this file contains your ESP board, make sure your board is not commented out, and that all other boards are commented out, and save it. This should be a good `platformio.ini` file for SensESP. If your board is NOT included, you need to add it (see how it's represented in the `platformio.ini.auto` that you saved). And you need to merge the other lines in `platformio.ini.auto` into your `platformio.ini` file. (Detailed instructions for merging these two files are available [in the Wiki](https://github.com/SignalK/SensESP/wiki/SensESP-Overview-and-Programming-Details#getting-a-good-platformioini-file).)

NOTE - ESP32 ONLY: A bug was introduced in PlatformIO Core Version 5.0 that doesn't properly pull in all library dependencies in all situations. ([Here](https://github.com/platformio/platformio-core/issues/3729) is the Issue.) If you are using an ESP32, you must build for an ESP8266 first, or the ESP32 build will fail. This is true only for a truly "clean" build: no `~/.platformio` directory, and no `.pio` directory in your Project directory. Here is the work-around, which you must do for your first Build of a Project, and any time after you delete `~/.platformio` and your Project's `.pio` directory.
1. In the `[platformio]` section of your Project's `platformio.ini`, comment out all but `d1_mini`.
2. Build the Project.
3. Comment out `d1_mini`, and un-comment the ESP32 board you're building for.

Hopefully this bug will be fixed soon in PlatformIO.

Now, open `src/main.cpp`. The default template is for the Arduino IDE, but a SensESP `main.cpp` file will look very different. Replace the contents of `main.cpp` with the contents of one of the SensESP examples in the examples subdirectory here on GitHub. (This is a good one to start with: https://github.com/SignalK/SensESP/blob/master/examples/analog_input.cpp .) Check that the settings (pin numbers, etc.) match your hardware. Then click on the checkmark icon on the blue status bar along the bottom of your screen. (That's the "Build" shortcut.) If the build succeeds, you can plug in your ESP board and press "Upload and Monitor".

If the project compiles and uploads, your ESP will be running the example code. Since the first thing it needs to do is connect to a wifi network, and it doesn't know what network to connect to, it will broadcast a wifi SSID for you to connect to so you can configure it. Connect your computer or phone wifi to the "Configure SensESP" network. A captive portal may pop up, but if it doesn't, open a browser and go to 192.168.4.1. Enter your wifi credentials to allow the device to access the network that your Signal K Server is on. Also enter a suitable name for the ESP, for example `BilgeMonitor` or `EngineTemps`. (No more than 16 characters, no spaces.) Save the configuration with the button on the bottom of the page, and the ESP will restart and try to connect to your wifi network.

Once on the network, SensESP should automatically find your Signal K server, assuming it has mDNS enabled (see instructions above). 

If your server has security enabled (it does by default), you should see an access request for your ESP in the Signal K Dashboard, under Security - Access Requests. (You must be logged into the Signal K Server to see the Security sub-menu.) Set the "Authentication Timeout" field to "NEVER", set the Permission to "Read / Write", then Approve it. You should start getting data on the Signal K Instrument Panel. (Dashboard - Webapps - Instrument Panel) You can see lots of activity in the Serial Monitor, including the connection to the Signal K Server, the request for and approval of the security token, and the flow of data.

If you have any problems with configuring the wifi credentials, or with SensESP finding your Signal K server, you can hard-code those settings. See [Hard-coding Certain Program Attributes](https://github.com/SignalK/SensESP/wiki/SensESP-Overview-and-Programming-Details#hard-coding-certain-program-attributes).

Some additional instructions are available on the [SensESP wiki](https://github.com/SignalK/SensESP/wiki/). 

## Low-level "wiring"

This refers to how you actually use the SensESP library to "wire" (in software) a sensor to send its output to the Signal K Server. Rather than explain it here, just look at the examples - most of them have a lot of comments that describe how this works. Showing is better than telling in this case.

## Runtime setup

You can configure your device with any web browser by putting the IP address of your device into the URL field of the browser, or by going to

    http://sensesp.local (where "sensesp" is the name you gave your device when you configured the wifi credentials)

You can find the IP address of your device in the Serial Monitor, right after the line showing that it has connected to your wifi.     

Everything that is configurable on a "live" device will be in the menu that appears. As you'll see in the examples, this includes things like how often you want to read the sensor (usually represented as "read_delay"), how many samples you want in the MovingAverage() transform, what multiplier and offset you want in the Linear() transform, etc., etc. You can also rename the device from that menu, and restart the device, and even reset the device to factory settings (which erases the wifi credentials and the device name).

## Getting help

Discussion about SensESP happens mostly in [Signalk-dev Slack](http://slack-invite.signalk.org/) on the #sensors channel. Don't hesitate to join and ask if you ever have problems with getting your project working!

## Contributing

Some guidelines about contributing to SensESP are given on the [SensESP wiki](https://github.com/SignalK/SensESP/wiki/Contributing-to-the-SensESP-Project). In particular, you should follow the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html) and try to keep the Git commits clean. The project maintainers are active on the [Signalk-dev Slack](http://slack-invite.signalk.org/) #sensors channel. It's recommended to provide a heads-up about your plans there first!

## SensESP Class Diagram

----------------------------

![alt text](sens_esp_uml.png "UML for SensESP")
