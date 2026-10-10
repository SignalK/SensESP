---
layout: default
title: Outlook Guardian
nav_order: 15
has_children: true
---

# Signal K Outlook Guardian

The Outlook Guardian is a talking alarm annunciator for boats.
It is an ESP32-S3 device that listens for vessel alarms and reads them out loud, so that the crew hears *what* is wrong and not just a buzzer.

It is built with SensESP and lives in this repository as the application [`examples/outlook_guardian.cpp`](https://github.com/OpenFairWind/S3-Outlook-Guardian/blob/main/examples/outlook_guardian.cpp).

## What it does

- **Speaks Signal K notifications.** It subscribes to `notifications.*` on the Signal K server and speaks the `message` of every raised notification.
- **Falls back to NMEA 2000.** When the Signal K server is not connected, it listens to the NMEA 2000 alert messages directly on the bus, composes a sentence from each alert and speaks it.
- **Shows the alarm on the RGB LED.** The color follows the most severe alarm, and the LED blinks until somebody reacts.
- **Has two push buttons**, one to acknowledge and one to snooze. For Signal K notifications the button press is sent to the server, so the other displays on board follow.
- **Speaks six languages.** The language, voice, speed and volume are chosen in the web interface.
- At startup it says "Signal K Outlook Guardian active".

Speech is synthesized on the device with the [PicoTTS](https://github.com/DiUS/esp-picotts) engine and played through a MAX98357A I2S amplifier. No internet connection or cloud service is involved.

## Where to go next

| Page | Contents |
|------|----------|
| [Hardware](hardware) | Parts, wiring and pin assignments |
| [Building and flashing](building) | Getting the firmware onto the board |
| [Using the device](usage) | First start, settings, LED, buttons |
| [Alarm sources](alarm_sources) | How Signal K notifications and NMEA 2000 alerts are handled |
| [Development notes](development) | Code structure, languages, flash layout |

## Quick start

1. Wire the amplifier, the CAN transceiver and the two buttons as described in [Hardware](hardware).
2. Build and flash the `pioarduino_esp32s3` environment as described in [Building and flashing](building).
3. Connect to the `outlook-guardian` WiFi access point and enter your boat's WiFi settings.
4. Approve the device's access request on the Signal K server, with **read/write** permission.
5. Open `http://outlook-guardian.local/` and choose the language and voice under *Speech and alarms*.
