---
layout: default
title: Building and flashing
parent: Outlook Guardian
nav_order: 20
---

# Building and flashing

## Prerequisites

- [PlatformIO](https://platformio.org/), either the Visual Studio Code extension or the command line tool.
- Git.

## Getting the source

The PicoTTS engine is a git submodule, so clone with `--recursive`:

```bash
git clone --recursive https://github.com/OpenFairWind/S3-Outlook-Guardian.git
```

If you have already cloned without it:

```bash
git submodule update --init
```

## Building

Build the `pioarduino_esp32s3` environment. In Visual Studio Code, pick it in the PlatformIO toolbar and press *Build*. On the command line:

```bash
pio run -e pioarduino_esp32s3
```

SensESP itself is a library with no `setup()` or `loop()`, so this environment also compiles one application from the `examples` directory. Which one is set by `custom_example` in `platformio.ini`:

```ini
[env:pioarduino_esp32s3]
custom_example = outlook_guardian
```

Other values that work on this hardware are `picotts_app`, `picotts_shallow_water` and `max98357a_tone`, which are small test programs for the speech engine and the amplifier.

The first build takes a few minutes. The PicoTTS sources produce a number of compiler warnings, which are harmless.

## Flashing

Connect the board with a USB cable and upload:

```bash
pio run -e pioarduino_esp32s3 -t upload
```

The firmware is about 8.3 MB, so the upload takes a bit over a minute.

On the first start after flashing, the device formats its configuration filesystem. This takes a few seconds, during which the serial log fills with `task_wdt: esp_task_wdt_reset` messages. They stop once the format is done and do not come back.

To watch the log:

```bash
pio device monitor -e pioarduino_esp32s3
```

## Firmware updates

Update the firmware over USB. Over-the-air updates are **not available**: with all six languages the firmware does not fit twice in 16 MB of flash, so there is no second partition to receive an update. See [Development notes](development) for how to get OTA back by building fewer languages.
