# SensESP

SensESP is a Signal K sensor development library for ESP8266
and ESP32. It can be used as a high-level toolkit for
creating ESP-based hardware sensoring devices that interface with Signal K
servers.

Class documentation for SensESP is [here](http://signalk.org/SensESP/generated/docs/annotated.html).

A Wiki page with more detailed information about using SensESP is [here](https://github.com/SignalK/SensESP/wiki).

## Important News

As of now (Nov. 2021), SensESP version 2 is under heavy development and will be released at some point in the near future.
This major release will have backwards-incompatible changes and will only support the ESP32 platform.
If you are only starting your SensESP journey now, only use ESP32 devices!

Version 1.0.x releases will be indefinitely available and can be used to support your existing projects, also on ESP8266.
To peg the SensESP version, use the following (or similar) line in your `platformio.ini` file `lib_deps` section:

    SignalK/SensESP @ ^1.0.7

## Getting Started

You must have a Signal K Server running on your network, or SensESP has nothing to connect to. The most common installation is the Signal K node server running on a Raspberry Pi. Installation instructions for that are [here](https://github.com/SignalK/signalk-server-node/blob/master/raspberry_pi_installation.md).

NOTE: SensESP does not currently support SSL. If you enabled that when you installed your Signal K Server, you will need to disable it before proceeding. From a command line on your Signal K Server's Pi, run `sudo signalk-server-setup`. It will go through all the setup options again, and when you get to the SSL option, turn it OFF, then finish the setup script.

Once the SK Server is installed and running, go to the Dashboard (enter `localhost:3000` into the Raspberry Pi's browser to start it), select Server - Settings from the left side menu, and make sure the "mdns" option is ON.

SensESP is a library and will be installed automatically as a dependency when defined as such in the project's
`platformio.ini` file. Instructions below. However, in order for PlatformIO to install all of the libraries that SensESP uses, you must have Git installed. If you don't, install it now: https://git-scm.com/book/en/v2/Getting-Started-Installing-Git.

You need to have PlatformIO installed. First, download and install [Visual Studio Code](https://code.visualstudio.com/).
Then, select "Extensions" from the left toolbar. Search for "platformio", select the first result, and click "Install".

Once you have PlatformIO installed, its home screen should open automatically when you start VSCode. On the PIO Home, 
select "New Project". Enter a name (something like SensESPTest for your first project) and then select "Espressif ESP32 Dev Module" in the board dropdown. (This assumes you're using an ESP32 based development board. If you're using a different ESP, select that in the board dropdown.)
The Arduino framework should become automatically selected. Complete the New Project dialog, then open the project you created.

Once you have your new project open, open the automatically generated `platformio.ini` file that's in your project's directory (NOT the one that you find if you go down into the .pio/libdeps/... folders). Save it as `platformio.ini.auto` so you can refer back to it.

Create a new file, and copy/paste the entire contents of [this example `platformio.ini` file](https://github.com/SignalK/SensESP/blob/master/examples/platformio.ini) into it. Save it as `platformio.ini`.

If the `[platformio]` section of this file contains your ESP board, make sure your board is not commented out, and that all other boards are commented out, and save it. This should be a good `platformio.ini` file for SensESP. If your board is NOT included, you need to add it (see how it's represented in the `platformio.ini.auto` that you saved). And you need to merge the other lines in `platformio.ini.auto` into your `platformio.ini` file. (Detailed instructions for merging these two files are available [in the Wiki](https://github.com/SignalK/SensESP/wiki/SensESP-Overview-and-Programming-Details#getting-a-good-platformioini-file).)

Now, open `src/main.cpp`. The default template is for the Arduino IDE, but a SensESP `main.cpp` file will look very different. Replace the contents of `main.cpp` with the contents of one of the SensESP examples in the examples subdirectory here on GitHub. (This is a good one to start with: https://github.com/SignalK/SensESP/blob/master/examples/analog_input.cpp .) Check that the settings (pin numbers, etc.) match your hardware. Then click on the checkmark icon on the blue status bar along the bottom of your screen. (That's the "Build" shortcut.) If the build succeeds, you can plug in your ESP board and press "Upload and Monitor".

If the project compiles and uploads, your ESP will be running the example code. (If you get errors about missing libraries, look at [this Troubleshooting guide in the Wiki](https://github.com/SignalK/SensESP/wiki/Troubleshooting#missing-libraries)). Since the first thing it needs to do is connect to a wifi network, and it doesn't know what network to connect to, it will broadcast a wifi SSID for you to connect to so you can configure it. Connect your computer or phone wifi to the "Configure SensESP" network. A captive portal may pop up, but if it doesn't, open a browser and go to 192.168.4.1. Enter your wifi credentials to allow the device to access the network that your Signal K Server is on. Also enter a suitable name for the ESP, for example `BilgeMonitor` or `EngineTemps`. (No more than 16 characters, no spaces.) Save the configuration with the button on the bottom of the page, and the ESP will restart and try to connect to your wifi network.

Once on the network, SensESP should automatically find your Signal K server, assuming it has mDNS enabled (see instructions above). 

If your server has security enabled (it does by default), you should see an access request for your ESP in the Signal K Dashboard, under Security - Access Requests. (You must be logged into the Signal K Server to see the Security sub-menu.) Set the "Authentication Timeout" field to "NEVER", set the Permission to "Read / Write", then Approve it. You should start getting data on the Signal K Instrument Panel. (Dashboard - Webapps - Instrument Panel) You can see lots of activity in the Visual Studio Code Serial Monitor, including the connection to the Signal K Server, the request for and approval of the security token, and the flow of data.

If you have any problems with configuring the wifi credentials, or with SensESP finding your Signal K server, you can hard-code those settings. See [Hard-coding Certain Program Attributes](https://github.com/SignalK/SensESP/wiki/SensESP-Overview-and-Programming-Details#hard-coding-certain-program-attributes).

Some additional instructions are available on the [SensESP wiki](https://github.com/SignalK/SensESP/wiki/). 

## Low-level "wiring"

This refers to how you actually use the SensESP library to "wire" (in software) a sensor to send its output to the Signal K Server. Rather than explain it here, just look at the examples - most of them have a lot of comments that describe how this works. Showing is better than telling in this case.

A frequent question from new SensESP users is "Can you have more than one sensor in a single project?". Yes, you can. See the [Multiple Sensors Example](https://github.com/SignalK/SensESP/blob/master/examples/multiple_sensors_example.cpp)

## Runtime setup

You can configure your device with any web browser by putting the IP address of your device into the URL field of the browser, or by going to

    http://sensesp.local (where "sensesp" is the name you gave your device when you configured the wifi credentials)

You can find the IP address of your device in the Serial Monitor, right after the line showing that it has connected to your wifi.     

Everything that is configurable on a "live" device will be in the menu that appears. As you'll see in the examples, this includes things like how often you want to read the sensor (usually represented as "read_delay"), how many samples you want in the MovingAverage() transform, what multiplier and offset you want in the Linear() transform, etc., etc. You can also rename the device from that menu, and restart the device, and even reset the device to factory settings (which erases the wifi credentials and the device name).

## Add-on libraries

Going forward, SensESP attempts to focus on providing the core "scaffolding" for building asynchronous and Signal K aware sensor devices. It will not include sensor classes for every possible piece of hardware but those can be provided by third parties and pulled in as dependencies as any other library. Thus, SensESP will hopefully become more like a network of affiliated projects.

A list of SensESP add-on libraries can be found on a separate page: [SensESP add-ons and related projects](https://github.com/SignalK/SensESP/blob/v1/docs/Add-ons.md


## Getting help

Discussion about SensESP happens mostly in [Signalk-dev Slack](http://slack-invite.signalk.org/) on the #sensors channel. Don't hesitate to join and ask if you ever have problems with getting your project working!

## Contributing

Some guidelines about contributing to SensESP are given on the [SensESP wiki](https://github.com/SignalK/SensESP/wiki/Contributing-to-the-SensESP-Project). In particular, you should follow the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html) and try to keep the Git commits clean. The project maintainers are active on the [Signalk-dev Slack](http://slack-invite.signalk.org/) #sensors channel. It's recommended to provide a heads-up about your plans there first!
