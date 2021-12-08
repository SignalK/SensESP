---
layout: default
title: Getting started
nav_order: 20
---

# Getting started

- Getting started with an example project
- Supported hardware
- Basic program structure

SensESP is a library of functions that makes it relatively simple to create a program that will run on an ESP8266 or ESP32 microcontroller ("MCU"), that collects data from various sensors, processes the data, and sends it along to a final "consumer", typically a Signal K server. (See [Sensors](#sensors) and [Transforms](#transforms) below.)

It must be used with the PlatformIO IDE - it won't work with the Arduino IDE.

As the end user, you create a new project in PlatformIO and then modify its `platformio.ini` and `main.cpp` files. When you create a new project, those two files will be very generic - don't use them. See the detailed instructions below for getting these two files properly set up.

The best way to learn how to use SensESP is by looking at a few examples. In the `/examples` folder, `rpm_counter.cpp` is a good one to start with, as it illustrates a simple implementation - reading the value of a sensor connected to a GPIO pin on the MCU, converting that value to hertz (the native unit of measurement for a tachometer in Signal K), then sending the value in hertz to the Signal K server.

Note: if you're interested in just using SensESP in a new project, you should NOT begin modifying files in a cloned or downloaded SensESP directory tree. Instead, the only files you'll be working with are your Project's `main.cpp` and `platformio.ini`, following the instructions above. If you want to make changes or additions to the core SensESP functionality, see (https://github.com/SignalK/SensESP/wiki/Contributing-to-the-SensESP-Project).

### Getting a Good platformio.ini File
In your Project's main directory, there's a file called `platformio.ini` that contains all of the details that PlatformIO needs to know in order to build your project properly. When you create a new PlatformIO Project, a `platformio.ini` is created for you, based on the ESP you chose in the process of creating the new Project. That file won't work for SensESP. This section guides you through the details of getting a good `platformio.ini` for SensESP.

Once you have your new project open, open the `platformio.ini` file that's in your project's directory (NOT the one that you find if you go down into the .pio/libdeps/... folders). Save the file as "platformio.ini.auto".

Use File_New File to create a new, empty file. Copy-paste the entire contents of [the example platformio.ini file](https://github.com/SignalK/SensESP/blob/master/examples/platformio.ini) into this new file. Then File_Save this new file as "platformio.ini". Near the top of that file is a section that looks something like this:
```c++
[platformio]
;set default_envs to whichever board(s) you use. Build/Run/etc processes those envs
default_envs =
   ;esp32dev
   d1_mini
;   esp-wrover-kit
```
See if your board is one of the boards in the list. (Look in the file you saved as `platformio.ini.auto` to see how your board is represented.) If your board is in that list, make sure that it's NOT commented out (that is, make sure the line it's on doesn't start with a semi-colon), and make sure that all the other boards in that section ARE commented out. You should now have a good `platformio.ini`, and can move on to working with `main.cpp` (see below).

If your board is NOT in that list, you need to do a few things:
1. Add your board to the list. You'll know what to add by looking at the `board = ` section of `platformio.ini.auto`.
2. Comment out the other boards in the list.
3. Copy-paste the entire contents of `platformio.ini.auto` into the very bottom of `platformio.ini`. That will look something like this:
```c++
[env:your_board_name]
platform = espressif32
board = your_board_name
framework = arduino
```
4. Copy one of these on a new line immediately after `[env:your_board_name]`: if your board is an ESP8266, copy this: `extends = espressif8266_base`; if your board is an ESP32, copy this: `extends = espressif32_base`.
5. Comment out, or simply remove, the `platform = ` line. (It's already included in the "base" section that the previous line points to.)
6. Comment out, or simply remove, the `framework = arduino` line. (It's already included in the global `[env]` section earlier in the file.)
7. If there are other lines in the `[env:your_board_name]` section of `platformio.ini.auto`, you should leave them.
8. If there are other sections (besides the `[env:your_board_name]` section) in `platformio.ini.auto`, they're there because PlatformIO thinks you need them, so you should copy them into the bottom of `platformio.ini`, too.
9. Look through the entire `platformio.ini` file, looking at all of the comments, to determine if any of them pertain to your board or your project.

Now you should have a `platformio.ini` that will work for your board, and that has all the settings that have been determined to be necessary for SensESP.

Note that the `platformio.ini` file you now have points to the most recent "Release" version of SensESP. (`lib_deps = SignalK/SensESP`) It's the version that's in the branch called "latest" in the GitHub repo. If you want to use the `master` branch, which has all of the most recently-merged Pull Requests, you need to change that to `lib_deps = https://github.com/SignalK/SensESP`.

### Working with main.cpp
In a Platformio Project, the primary source code file is called `main.cpp` (not `YourProject.ino` like it is in the Arduino IDE). When you create a new Project, PlatformIO creates a `main.cpp` for you, but you can't use it for SensESP.

Open `/YourProjectName/src/main.cpp`. The default template is for the Arduino IDE, but a SensESP `main.cpp` file will look very different. Replace the contents of `main.cpp` with the contents of one of the SensESP examples in https://github.com/SignalK/SensESP/tree/master/examples. (This is a good one to start with: https://github.com/SignalK/SensESP/blob/master/examples/analog_input.cpp.) Check that the settings (pin numbers, etc.) match your hardware. Then click on the checkmark icon on the blue status bar along the bottom of your screen. (That's the "Build" shortcut.) If the build succeeds, you can plug in your ESP board and press "Upload and Monitor".

### Hard-coding Certain Program Attributes
All of the serious work of a SensESP program happens within an object called the SensESPApp. Prior to August 2020, every `main.cpp` file created an instance of that object with a line like this: `sensesp_app = new SensESPApp();`. It was decided to add the ability to set certain program attributes directly in main.cpp by using one of two different approaches: with the SensESPApp constructor, or using the C++ "builder pattern". Now, although the old way still works, you have the option to use either of these new approaches, which you'll see in some of the example files.

```c++
// Option 1: Create the SensESPApp with some program attributes passed as parameters
// to the SensESPApp constructor:

sensesp_app = new SensESPApp("sensesp-illum-example", "My WiFi SSID",
                               "my_wifi_password", "skdev.lan", 80, NONE);
```

The parameters to the constructor, including their default values, are:
```c++
SensESPApp(String hostname = "SensESP", String ssid = "",
             String wifi_password = "", String sk_server_address = "",
             uint16_t sk_server_port = 0);
```

See below for a description of each of them.

```c++
// Option 2: Create a SensESPAppBuilder object, then set some or all of the program
// attributes. Note that if you use the "builder", you do NOT specifically create an instance
// of the SensESPApp - that's done by the builder when you call "get_app()" on the last line.
// Note also that there are more attributes you can set with the "builder" than by passing
// parameters to the SensESPApp constructor.

  SensESPAppBuilder builder;

  // Set whatever options you want, then create the global SensESPApp() object with get_app():
  sensesp_app = builder.set_standard_sensors(UPTIME)
              ->set_hostname("SensESPtest")
              // ->set_sk_server("10.10.10.1", 3000) // Don't specify server address or port
              ->set_wifi("boatWifi", "ahoyMatie!")
              ->set_led_pin(13);
              ->set_led_blinker(true, 1000, 2500, 4000);
              ->get_app();
```
You can use as many of the "set" options as you want, and either comment out, or just not use, any of the others. For example, you may want to specify the "standard sensors" you want to see in Signal K (see below) and the wifi credentials, like this:
```c++
sensesp_app = builder.set_standard_sensors(UPTIME)
              ->set_wifi("boatWifi", "ahoyMatie!")
              ->get_app();
```
Here are all of the builder's "set" options, explained (code is in [sensesp_app_builder.h](https://github.com/SignalK/SensESP/blob/master/src/sensesp_app_builder.h)):
- `set_wifi(String ssid, String password)` - sets the SSID and password of the wifi network you want the MCU to connect to. If you don't use `set_wifi`, SensESP will create a wifi access point called "Configure Sensesp" for you to configure your wifi credentials. Unless your wifi SSID and/or password is expected to change, it's easier to hard-code it with `set_wifi`(If you use `set_wifi`, you won't be able to edit these things in the normal Configuration UI later. To change them, you'll need to edit `main.cpp`, then build and upload again.).

- `set_sk_server(String address, uint16_t port)` - sets the IP address and port on which your Signal K Server can be found. See above for example usage. If you don't use `set_sk_server`, SensESP will attempt to use mDNS to locate the server, but there have been some issues with this working 100% reliably, which is why this option was added.

- `set_standard_sensors(StandardSensors sensors = ALL)` - see below.

- `set_led_pin(int led_pin)` - sets the pin that will be used by `led_blinker` to indicate the network status. For an ESP8266, this will default to the LED_BUILTIN pin on the MCU. ESP32's don't have an LED_BUILTIN defined - consult the documentation for your particular ESP32 to see if there's an LED pin you can use.

- `set_led_blinker(bool enabled, int websocket_connected, int wifi_connected, int offline)` - sets the time, in miliseconds, that each of the three "network status" indicators flash on and off. In the first example above, the LED will flash on and off every 1 second to indicate "websocket connected"; every 2.5 seconds for "wifi connected", and every 4 seconds for "offline". If you don't use `set_led_blinker`, the defaults are 200, 1000, and 5000 ms, respectively.

- `set_hostname(String hostname)` - sets the name of the ESP Device, to appear in various places in Signal K. If you don't use `set_hostname`, it will default to "SensESP". If you do use `set_hostname`, it will be hard-coded, and not editable in the Configuration UI.

### Standard Sensors
The "standard sensors" aren't really sensors. They're data about the MCU that can be gathered by SensESP and sent to Signal K in a way that makes them look like the data from a "real" sensor. In part, this was done to give you some data to see from SensESP as soon as you get it running on your MCU, to see that it's all working. But some of the data could be useful to see in Signal K, such as UPTIME (to give you a quick way to know that a particular MCU is still connected and sending data). Here are the options - as of this writing, you may choose only one of them. (If you don't use `set_standard_sensors`, you will get all of them.) See [Hard-coding Certain Program Attributes](#hard-coding-certain-program-attributes) for instructions on how to set your selection.

- UPTIME - number of seconds that the MCU has been running since last restart
- FREQUENCY - clock speed of the MCU's microprocessor
- FREE_MEMORY - remaining free memory on the MCU
- IP_ADDRESS - IP address of the MCU on the network
- WIFI_SIGNAL - wifi signal strength
- ALL - all of the above - this is the default
- NONE - none of the above

### Pin Numbers
ESP8266 pin numbers are referred to in code like this: D1, D2, etc.
ESP32 pin numbers are referred to in code as integers: 1, 2, etc.
In each case, make sure you're using the correct number for the pin, by looking at a pin layout diagram for your particular MCU.

### Sensors
A "Sensor" in SensESP is a class that knows how to get data from an actual, physical sensor and pass it along to a Transform. For example, an AnalogInput Sensor knows how to read the AnalogIn pin on the MCU, and a DigitalInputCounter Sensor knows how to read a GPIO pin on the MCU using an interrupt. The combination of a BME280 Sensor (the software) and a BME280value Sensor (also software) knows how to read the temperature, humidity, and pressure from a phyical Adafruit BME280 sensor.

Code to read from many sensors is built-in to SensESP. You can browse a list of them [here](https://github.com/SignalK/SensESP/tree/master/src/sensors). Code to read even more sensors can be found [here](https://github.com/SensESP).

### Transforms
A Transform is a class that takes a value as input (from a Sensor, or from another Transform), does something with that value (transforms it), then outputs the new value, either to another Transform, or to the Signal K server. In the `rpm_counter.cpp` example referred to above, the meat of the code is these three lines:
```c++
auto* sensor = new DigitalInputCounter(D5, INPUT_PULLUP, RISING, read_delay);

sensor
    ->connectTo(new Frequency(multiplier, config_path_calibrate))
    ->connectTo(new SKOutputNumber(sk_path, config_path_skpath));
```
The first line instantiates a Sensor of type DigitalInputCounter. The second line is a Transform of type Frequency - it takes a raw number from the DigitalInputCounter Sensor and converts it to hertz, then passes it along to SKOutputNumber. SKOutputNumber is a special Transform whose purpose is to send a value to the Signal K server.

A much more complex example is `temperature_sender.cpp`, where the meat of the program is this:
```c++
auto* analog_input = new AnalogInput();

analog_input->connectTo(new AnalogVoltage())
    ->connectTo(new VoltageDividerR2(R1, Vin, "/gen/temp/sender"))
    ->connectTo(new TemperatureInterpreter("/gen/temp/curve"))
    ->connectTo(new Linear(1.0, 0.0, "/gen/temp/calibrate"))
    ->connectTo(new SKOutputNumber(sk_path, "/gen/temp/sk"));
```
In this example, there is still only one Sensor (AnalogInput), but several Transforms, all required to turn the raw value from the Analog Input pin on the MCU into a temperature that's sent to the Signal K server.

You can also include multiple Sensors, each with at least one Transform, in the same program, such as including both of the examples above into the same `main.cpp`, one after the other. (TODO - add such an example to /examples.)

### Configuration Paths
Some Sensors and Transform have parameters that can be configured "live", by accessing the MCU through its IP address, entered as a URL in any browser. For example, entering `192.168.1.236` (the IP address of the MCU I'm currently using as my example) in a browser will bring up the following web page that's hosted by the MCU itself:

![image](https://user-images.githubusercontent.com/15186790/76421208-b52c6680-6379-11ea-9470-a4b01c984927.png)

"Device information" displays some information about the MCU, the firmware, the wifi network, etc..
"Configure device" is explained below.
"Restart device" will restart the MCU.
"Reset device" will erase all the network information and all the Sensor and Transform configuration, but not the program. The next time the MCU boots, all configurable values will be back at the defaults set in the program.

Click on "Configure device" to bring up the /setup page, which will look something like this:

![image](https://user-images.githubusercontent.com/15186790/76452887-33076680-63a8-11ea-99d6-19ada7b53c64.png)

The only menu item that will always be present is "system", which has two sub-menus: "networking" (which lets you see and edit your wifi SSID and password, and the MCU's hostname), and "sk", which displays some networking info, but can't yet be used for editing that info.

The other menu item(s) (in this example, only "blackWater") are there only if you've provided a configuration path in the constructor of one or more of the Sensors and Transforms in `main.cpp`. Naming of these is important, and covered below.

Look at the three lines from the `rpm_counter.cpp` example above. There are three constructors - one for the DigitalInputCounter Sensor, one for the Frequency Transform, and one for the SKOutputNumber Transform. The latter two have a configuration path included as the last item in their parameter list, but the first one doesn't. That means that the first one - the DigitalInputCounter - has no values that can be configured "live", but the latter two do. Actually, it's a little more complicated than that, because it's not entirely consistent among all Sensors and Transforms.

As of this writing (August 2020), here are the ** possible ** configurable parameters for each Sensor and Transform:

** SENSORS **
- ADS1x15: Read delay (Number of milliseconds between each read of the sensor)
- AnalogInput: Read delay
- BME280value, BMP280value, INA219value, MAX31856TC, SHT31value, UltrasonicSens: Read delay
- DigitalInputValue, DigitalOutput: none
- DigitalInputCounter: Read delay
- GPSInput: none
- OneWireTemperature: OneWire address (the unique address of the specific sensor being used)

** TRANSFORMS **
- AnalogVoltage: Max voltage of the MCU, Multiplier, Offset
- AngleCorrection: Constant offset (value to be added, in radians); Minimum angle value (pi, or 0)
- ChangeFilter: Minimum delta (Minimum difference in change of value before forwarding); Maximum delta (maximum difference in change of value to allow forwarding); Max skips (maximum number of consecutive filtered values before one is allowed through)
- CurveInterpolator: input, output (each pair is added to the table of samples)
- Difference: Input #1 multiplier, Input #2 multiplier
- Frequency: Multiplier
- Integrator: Multiplier
- Linear: Multiplier; Constant offset
- Median: Sample size (number of samples to take before outputting a value)
- MovingAverage: Number of samples in average; Multiplier
- NumericThreshold: minValue, maxValue, inRange (the value to output when the input is between minValue and maxValue)
- VoltageDivider: Voltage in; Resistance (ohms) of R1
- VoltageMultiplier: Ohms values of R1 and R2
- SKOutputNumber, SKOutputInt, SKOutputBool, SKOutputString: Signal K data path

In every case of a configurable value, that value can be set in `main.cpp` with a parameter to the constructor of the Sensor or Transform. By making it configurable, it's easier to make adjustments to your output based on what you're seeing in the real world. For example, the Median Transform is used to smooth output from a "noisy" sensor. By making the "Sample size" configurable, you can experiment with different sample sizes while the MCU is running and outputting data to Signal K, so you can decide when you have the right sample size for your purposes.

Even though a Sensor or Transform has the ability to be configurable, it won't be unless you provide a configuration path in the constructor when you use it in `main.cpp`. For example,
```c++
auto* analog_input = new AnalogInput();
```
creates an AnalogInput Sensor whose Read Delay will be the default 200 ms, and which can't be configured.
```c++
auto* analog_input = new AnalogInput(250);
```
creates a Sensor with a 250 ms Read Delay that still can't be adjusted in real time, because of the lack of a config_path parameter.

But
```c++
auto* analog_input = new AnalogInput(250, "/analogInput");
```
creates a Sensor with a 250 ms Read Delay that can be adjusted in real time, because of the presence of the config_path parameter (`"/analogInput"`).

Your configuration path parameter can be passed with a variable you create, like this:
```c++
const char* sensor_config_path = "/analogInput";
auto* analog_input = new AnalogInput(250, sensor_config_path);
```
or by putting the configuration path String directly into the parameter list of the constructor, like this:
```c++
auto* analog_input = new AnalogInput(250, "/analogInput");
```

### Naming Configuration Paths
The naming of the paths is important, especially when you have multiple Sensors and / or multiple Transforms in your `main.cpp`, so to be safe, please follow these guidelines:
- Every configuration path name MUST begin with a forward slash.
- Use two levels in your names, so that they look like `"/firstLevel/secondLevel"`, with the first level being a word that groups entries together in a logical manner, and the second level referring to the specific Sensor or Transform that the configuration path relates to. For example:

Two Sensors (one for black water and one for fresh water), each using a Moving Average Transform and outputting to the Signal K Server with SKOutputNumber:
- "/blackWater/analogInput" (for the blackwater AnalogInput() constructor in `main.cpp`)
- "/blackWater/movingAvg" (for the blackwater MovingAverage() constructor in `main.cpp`)
- "/blackWater/skPath" (for the blackwater SKOutputNumber() constructor in `main.cpp`)
- "/freshWater/analogInput" (for the fresh water AnalogInput() constructor)
- "/freshWater/movingAvg" (for the fresh water MovingAverage() constructor)
- "/freshWater/skPath" (for the fresh water SKOutputNumber() constructor)

This will group the configuration entries in the web interface into two groups: "blackWater" and "freshWater". Each group will have three entries: "analogInput", "movingAvg", and "SKOutput". Each "analogInput" entry will have one configurable value: "Read delay"; each "movingAvg" entry will have two configurable values: "Number of samples" and "Multiplier"; and each "skPath" entry will have one configurable value: "SignalK Path".

### Troubleshooting

SensESP is a complex library and unfortunately, library dependency issues are not uncommon, especially if you've had the project around for a while and the upstream dependencies have changed. The first thing you should _always_ try is deleting your local `.pio` directory within your project directory. That will make PlatformIO re-download and recompile all dependency libraries and is often enough to resolve problems.

Also, see the [Troubleshooting page of the Wiki](https://github.com/SignalK/SensESP/wiki/Troubleshooting).

Also, join the Signalk-dev Slack and ask on the `#sensors` channel.
