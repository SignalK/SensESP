---
layout: default
title: Features
nav_order: 40
---

# Features

## Button Handling

Commonly used ESP32 devices such as the [SH-ESP32](https://docs.hatlabs.fi/sh-esp32/) or the DevKitC variants have a press button on the GPIO pin 0. SensESP by default recognizes the following button presses:

- Short press (less than 1 second): Restart the device
- Long press (more than 1 second): Reset the network settings
- Very long press (more than 5 seconds): Reset the device to factory settings

If you want to use a different pin for the button, add the following line to the Builder section in main.cpp, before the `->get_app()` instruction:

```c++
->set_button_pin(15)
```
where the new button GPIO would be 15 in this example.

If you want to disable the button handling completely, add the following line to the Builder section in main.cpp, before the `->get_app()` instruction:

```c++
->set_button_pin(-1)
```

It is also possible to define completely custom button behavior by disabling the default button as above and then adding your own button handler class that inherits from `BaseButtonHandler` or `MinimalButtonHandler`.

## OTA updates

Activating Over The Air (OTA) updates, requires a 2 Step setup:

STEP 1:
1. Add the line

   ```c++
    ->enable_ota("SomeOTAPassword")
   ```

   to the Builder section in main.cpp, before the `->get_app()` instruction. Nothing else.

2. Upload `main.cpp` to the ESP via USB link, which will still work as usual.


STEP 2:
1. Add the following lines to platformio.ini, under the `[env:esp32dev]` section:

   ```ìni
     upload_protocol = espota
     upload_port = "ip address of esp"  (mDNS might not work)
     upload_flags =
         --auth=SomeOTAPassword
   ```

2. Upload `main.cpp` again. This will now happen OTA irrespective of whether USB cable is connected or disconnected.

----

## WiFi manager
## System info sensors
