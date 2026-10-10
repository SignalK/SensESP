---
layout: default
title: Development notes
parent: Outlook Guardian
nav_order: 50
---

# Development notes

## Files

| File | Contents |
|------|----------|
| `examples/outlook_guardian.cpp` | The application |
| `examples/outlook_guardian_16MB.csv` | Flash partition table |
| `src/sensesp/system/pico_tts.h` | `PicoTTS`: queued text-to-speech output |
| `src/sensesp/system/max98357a.h` | `MAX98357A`: I2S amplifier driver |
| `lib/picotts/` | PlatformIO wrapper around the `esp-picotts` submodule, with support for several built-in languages |
| `platformio.ini` | The `[esp32s3]` board section and the `pioarduino_esp32s3` environment |

## Application structure

`outlook_guardian.cpp` is organised around one table of alarms, filled by either source:

- `GuardianConfig` holds the settings shown in the web interface.
- `AlarmLed` replaces the SensESP system status LED and shows the alarm state.
- `handle_sk_notification()` turns a Signal K notification into an alarm.
- `handle_n2k_message()` collects the NMEA 2000 alert messages, and `raise_n2k_alarms()` turns them into alarms once their sentence can be composed.
- `process_alarms()` runs ten times a second. It wakes up snoozed alarms, picks the next alarm to speak and updates the LED.
- `acknowledge_alarms()` and `snooze_alarms()` are called by the buttons.
- `check_alarm_source()` switches between Signal K and NMEA 2000 as the server connection comes and goes.

Everything runs on the SensESP event loop. The speech synthesis runs in its own task inside `PicoTTS`.

## Languages

PicoTTS supports six languages, with one voice each. All of them are built in:

```ini
custom_picotts_languages = en-US, en-GB, it-IT, de-DE, es-ES, fr-FR
```

The first one is the default. Each language adds between 0.9 and 1.4 MB to the firmware.

The words used for the composed sentences are in the `kPhrases` table in `outlook_guardian.cpp`.

The voice variants are in the `kVoices` table. They are applied with PicoTTS markup: the text is wrapped in `<pitch level="…">` and `<speed level="…">` tags before it is handed to the engine.

## Flash layout and OTA

With all six languages the firmware is about 8.3 MB. Two copies do not fit in 16 MB, so the partition table has a single 10 MB application partition and over-the-air updates are not possible:

| Partition | Size |
|-----------|------|
| nvs | 20 KB |
| otadata | 8 KB |
| app0 | 10 MB |
| spiffs (configuration) | 5.9 MB |
| coredump | 64 KB |

To get OTA updates back, build fewer languages so that the firmware fits in 6.4 MB, and use the stock two-partition table. With two languages the firmware is about 4.1 MB:

```ini
board_build.partitions = default_16MB.csv
custom_picotts_languages = en-US, it-IT
```

Changing the partition table moves the configuration filesystem, so the device's settings, including WiFi, have to be entered again.

## Change to the SensESP framework

`SensESPAppBuilder::set_system_status_led()` and `SensESPApp::set_system_status_led()` accept any `BaseSystemStatusLed` instead of only the monochrome `SystemStatusLed`. This is what lets the application install its own RGB LED handler.

## Not verified

The firmware has been built, flashed and seen to start on an ESP32-S3-DevKitC-1 N16R8. The following have not yet been tested against real equipment: the audio output in each language and voice, the handling of live Signal K notifications including the button requests, and the reception of NMEA 2000 alerts.
