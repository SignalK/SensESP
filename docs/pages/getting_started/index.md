---
layout: default
title: Getting Started
nav_order: 20
---

# Getting Started

## Introduction

SensESP is a library of functions that makes it relatively simple to create a program for an ESP32 microcontroller ("MCU"), that collects data from one or more sensors, processes the data, and sends it along to some "consumer", typically a Signal K Server. It can also be used to monitor information coming from a Signal K Server and take action based on that information, such as turning on your anchor light when it gets dark outside.

It can work with virtually any kind of sensor that can work with an Arduino, usually without having to write more than a few lines of code. It can also work with sensors requiring more complex coding, although you may have to do that coding yourself. One of its great strengths is that it insulates you from the code that connects to your wifi and interfaces with your Signal K Server - those things just happen when SensESP runs!

The best way to learn how to use SensESP is by looking at a few examples. They won't be installed on your computer until after you've built your first Project (described below), but in the meantime, you can see all of them in the [examples folder in the GitHub repo](https://github.com/SignalK/SensESP/tree/master/examples). `rpm_counter.cpp` is a good one to start with, as it illustrates a simple implementation - reading the value of a sensor connected to a GPIO pin on the MCU, converting that value to hertz (the native unit of measurement for a tachometer in Signal K), then sending the value in hertz to the Signal K Server. But there are many other examples, illustrating many other concepts, so have a look at several of them.

If you're only interested in using SensESP in a new Project, DO NOT clone or otherwise download the SensESP GitHub repo to your computer. All of those files will be automatically downloaded by PlatformIO (the IDE you'll be using) when they're needed to compile and build your Project. The only files you'll be working with are your Project's `main.cpp` and `platformio.ini`, following the instructions below.

If you want to make changes or additions to the core SensESP functionality, [click here](get_in_touch/) for more information. (BAS: this link doesn't work.)

## Getting Ready to Get Started

You must have a Signal K Server running on your network, or SensESP has nothing to connect to[^1]. The most common installation is the Signal K node server running on a Raspberry Pi. Installation instructions for that are [here](https://github.com/SignalK/signalk-server-node/blob/master/raspberry_pi_installation.md). Be sure to turn SSL OFF as you go through the installation - SensESP won't work if it's enabled.

Once the SK Server is installed and running, go to the Dashboard (enter `localhost:3000` into the Raspberry Pi's browser to start it), select Server - Settings from the left side menu, and make sure the "mdns" option is ON.

To build a SensESP project and upload it to your ESP, you'll need to [install Visual Studio Code, then install the PlatformIO extension](https://platformio.org/install/ide?install=vscode). SensESP doesn't work with the Arduino IDE. (Throughout the documentation, there will be many references to PlatformIO, which is an extension to Visual Studio Code. So when you see something like "Start PlatformIO", it really means, "Start Visual Studio Code and get into the PlatformIO extension.")

[^1]: This is not strictly true - it's possible to use SensESP for something other than connecting to a Signal K Server, as described [here](https://github.com/SignalK/SensESP/blob/master/examples/minimal_app.cpp). However, the typical SensESP user will be connecting to a Signal K Server.

## Start a New Project in PlatformIO

1. Start Visual Studio Code.
2. On the far left edge, click on the icon that looks like an alien face. That will start the PlatformIO extension.
3. The PlatformIO: Quick Access menu will appear. Under "PIO Home", click on "Projects & Configuration", which will open the Projects view.
4. Click on the "Create New Project" button and the Project Wizard will open.
5. Give your new Project a name. If you know what it's going to be, like a Bilge Monitor, give it a descriptive name. Otherwise, call it something like SensESPTest. (No more than 32 characters, no spaces.)
6. Select a board. This refers to the microcontroller you'll be using. If you type `esp32` into the "Board" field, the list will show all the ESP32 boards that PIO works with. Select your specific board, unless you don't see it in the list, in which case select "Espressif ESP32 Dev Module".
7. The "Framework" field should change automatically to "Arduino". Leave it that way.
8. Click the "Finish" button, and your new Project will be created.
9. A new `platformio.ini` file should open in the editor, and in the file navigator, if you click on "src", you'll see that there is a `main.cpp` file. Double-click on that to open it in the editor, and now you'll be looking at the only two files you should need to work with to create your first SensESP project.

## Getting a Good platformio.ini File

Now, all you need to do is modify the `platformio.ini` and `main.cpp` files. Once you have your new project open, open the `platformio.ini` file that's in your Project's directory - the file that was auto-generated. Save the file as `platformio.ini.saved`.

Use File_New File to create a new, empty file. Copy-paste the entire contents of [the example SensESP platformio.ini file](https://github.com/SignalK/SensESP/blob/master/examples/platformio.ini) into this new file. Then File_Save this new file as "platformio.ini". Near the top of that file is a section that looks something like this:

```c++
[platformio]
;set default_envs to whichever board(s) you use. Build/Run/etc processes those envs
default_envs =
   esp32dev
```

Look in the file you saved as `platformio.ini.saved` to see how YOUR board is represented. If it's in there as `esp32dev`, your `platformio.ini` file should be good to go, and can move on to "Working with main.cpp", below.

If your board is NOT an `esp32dev`, you need to do a few things:

1. Replace `esp32dev` with the name of your board. You'll know how your board is represented by looking at the `board =` section of `platformio.ini.saved`.
2. Copy-paste the entire contents of `platformio.ini.saved` into the very bottom of `platformio.ini`. That will look something like this:

    ```c++
    [env:your_board_name]
    platform = espressif32
    board = your_board_name
    framework = arduino
    ```

3. On a new line immediately after `[env:your_board_name]`, copy this: `extends = espressif32_base`.
4. Remove the `platform =` line. (It's already in the "base" section that the previous line points to.)
5. Remove the `framework = arduino` line. (It's already in the global `[env]` section earlier in the file.)
6. If there are other lines in the `[env:your_board_name]` section of `platformio.ini.saved`, you should leave them.
7. If there are other sections (besides the `[env:your_board_name]` section) in `platformio.ini.saved`, they're there because PlatformIO thinks you need them, so you should leave them in your modified `platformio.ini`.
8. Look through the entire `platformio.ini` file, looking at all of the comments, to determine if any of them pertain to your board or your project.

Now you should have a `platformio.ini` that will work for your board, and that has all the settings that have been determined to be necessary for SensESP.

Note that the `platformio.ini` file you now have points to the most recent "Release" version of SensESP. (`lib_deps = SignalK/SensESP`) It's the version that's in the branch called `latest` in the GitHub repo. If you want to use the `master` branch, which has all of the most recently-merged Pull Requests since `latest` was published, you need to change that to `lib_deps = https://github.com/SignalK/SensESP`. The `latest` branch is the safest, but the `master` branch is the most up-to-date. It's also not really supported - only the `latest` branch is - so unless you know you need something that's in `master` that's not yet in `latest`, you should stick with `latest`.

## Working with main.cpp

In a Platformio Project, the primary source code file is called `main.cpp` (not `YourProject.ino` like it is in the Arduino IDE). When you create a new Project, PlatformIO creates a `main.cpp` for you, but you can't use it for SensESP.

Open `/YourProjectName/src/main.cpp`. The default file is for the Arduino IDE, but a SensESP `main.cpp` file will look very different. Replace the entire contents of `main.cpp` with the contents of one of the SensESP examples in <https://github.com/SignalK/SensESP/tree/master/examples>. (This is a good one to start with: <https://github.com/SignalK/SensESP/blob/master/examples/analog_input.cpp>.) Check that the settings (pin numbers, etc.) match your hardware. Then click on the checkmark icon on the blue status bar along the bottom of your screen. (That's the "Build" shortcut.) If the build succeeds, you can plug in your ESP board and press the right-arrow icon, which will upload the firmware to your ESP32 and start to run it. Also click on the icon that looks like a wall outlet plug - that will open the Serial Monitor window so you can see what the program is doing.

## After It's Built, Uploaded, and Running

If the project compiles and uploads, your ESP will be running the example code. (If you get errors about missing libraries, see the [Troubleshooting page](pages/troubleshooting) (BAS: this link doesn't work.)). Since the first thing it needs to do is connect to a wifi network, and it doesn't know what network to connect to, it will broadcast a wifi SSID for you to connect to so you can configure it. Connect your computer or phone wifi to the "Configure SensESP" network; the password is `thisisfine`. A captive portal may pop up, but if it doesn't, open a browser and go to 192.168.4.1. Enter the SSID and password of your boat's wifi to allow the device to access the network that your Signal K Server is on. Also enter a suitable name for the ESP, for example BilgeMonitor or EngineTemps. (No more than 32 characters, no spaces.) Save the configuration with the button on the bottom of the page, and the ESP will restart and try to connect to your wifi network.

Once on the network, SensESP should automatically find your Signal K Server, assuming it has mDNS enabled. If it doesn't connect, you can try to troubleshoot it, or you can hard-code the Server's IP address and port - see below.

If your Signal K Server has security enabled (it does by default), you should see an access request for your ESP in the Signal K Dashboard, under Security - Access Requests. (You must be logged into the Signal K Server to see the Security sub-menu.) Set the "Authentication Timeout" field to "NEVER", set the Permission to "Read / Write", then Approve it.

You should start seeing data from your ESP on the Signal K Data Browser (on the Server's main menu). You can also see lots of activity in the Visual Studio Code Serial Monitor, including the connection to the Signal K Server, the request for and approval of the security token, and the flow of data. Of course, the data is not likely to be valid, since you probably haven't yet connected anything to your ESP32. That's OK - if you've got data flowing at this point, you're ready to move on.

If you have any problems with configuring the wifi credentials, or with SensESP finding your Signal K Server, you can hard-code those settings, as explained in the next section.

At this point, if you have a successfully running SensESP example and it's sending data to your Signal K Server, you're ready to start a real Project, to do some real work. Assemble your ESP32 and the physical sensors, find an example that's close to what you want to do, copy-paste it into your `main.cpp`, and modify it to make it work. The comments in the examples should make it clear what you need to modify.

Be sure to read about [Sensors](#sensors) and [Transforms](#transforms) below - you'll need to know what they are and how they work.

## Hard-coding Certain Program Attributes

In all of the [example programs](https://github.com/SignalK/SensESP/tree/master/examples), you'll see something similar to this:

```c++
  SensESPAppBuilder builder;
  sensesp_app = builder.get_app();
  ```

or it may look more like this:

```c++
  SensESPAppBuilder builder;
  sensesp_app = builder
                  .set_hostname("main_engine_temps")
                  ->set_wifi("My WiFi SSID", "my_wifi_password")
                  ->set_sk_server("192.168.10.3", 80)
                  ->get_app();
```

In the first instance above, where nothing is set or enabled, SensESP will use some default values and exhibit some default behavior, as follows:

* The SensESP device will get the default hostname of `SensESP`, which is what will appear in many places throughout the Signal K ecosystem to identify this device.
* When SensESP runs, since it can't connect to wifi without knowing the wifi SSID and password, it will host a little webpage that you can access from your computer or phone to configure the wifi credentials. Once it successfully connects to your network, it will save those credentials for all future connection attempts.
* Once connected to your network, SensESP will use mDNS to try to find your Signal K Server to connect to it.

In the second instance above, you can see that the hostname, wifi credentials, and Signal K Server information are hard-coded. You may find this valuable:

* Your SensESP device will be named something meaningful, like `main_engine_temps` or `bilge_monitor`, rather than just "SensESP".
* You won't have to manually configure your wifi credentials. But be aware that if you hard-code the wifi credentials, and then later change your wifi SSID or password, you'll have to physically connect your computer to the ESP device to reflash it with the updated `main.cpp` that reflects the new SSID and password. Also, hard-coded credentials could accidentally be uploaded to a GitHub repo and exposed to the world. Be careful!
* mDNS *usually* works, but there are many components of it that are beyond the control of SensESP that could cause it to not work for you. If you encounter any issues connecting with it, you can try hard-coding the Signal K Server address and port.

In addition, there are other settings you may want to make with the above approach:

* `enable_ota(const char* password)` enables Over-The-Air firmware updates for your SensESP device.
* `set_wifi_manager_password(const char* password)` changes the password that allows you to set the wifi credentials from your phone or computer (if you don't hard-code the credentials as shown above).
* `set_system_status_led()` allows you to change the default blinking patterns of the ESP32's LED that indicate various conditions, like "wifi disconnected", "websocket connection authorizing", etc.
* `enable_wifi_signal_sensor()` sends the strength of the wifi signal, measured at the ESP32, to the Signal K Server as data that can be displayed by any other Signal K consumer, just like wind speed, oil pressure, etc.
* `enable_uptime_sensor()` sends the number of seconds since the last reboot of the ESP32 to the Signal K Server.
* `enable_free_mem_sensor()` sends the current free memory of the ESP32 to Signal K.
* `enable_system_hz_sensor()` sends the current clock speed of the ESP32 to Signal K.
* `enable_ip_address_sensor()` sends the IP address of the ESP32 to Signal K.
* `enable_system_info_sensors()` enables all five of the previous "sensors" with a single line.

You can use as many of the above `set / enable` options as you like. Just make sure that the first one is preceeded by a dot (like `builder.set_hostname`), that all the rest of them are preceeded by the right arrow (like `->set_wifi`), and that the last one is always `->get_app();`.
