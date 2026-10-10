---
layout: default
title: Using the device
parent: Outlook Guardian
nav_order: 30
---

# Using the device

## First start

1. Power the device. It says "Signal K Outlook Guardian active".
2. With no WiFi configured, it opens an access point called `outlook-guardian`. Connect to it and enter your boat's WiFi settings in the page that opens.
3. The device looks for the Signal K server on the network and asks it for access. In the Signal K server's admin interface, open *Security → Access Requests* and approve the request with **read/write** permission. Read/write is needed for the acknowledge and snooze buttons to reach the server.
4. Open `http://outlook-guardian.local/` to reach the web interface.

Without WiFi or without a Signal K server, the device still works from the NMEA 2000 bus alone.

## Settings

The settings are on the *Configuration* page of the web interface, in the *Speech and alarms* card.

| Setting | Default | Meaning |
|---------|---------|---------|
| Language | en-US | The language of the voice: `en-US`, `en-GB`, `it-IT`, `de-DE`, `es-ES` or `fr-FR`. |
| Voice | Standard | *Standard*, *Deep* or *High*. PicoTTS has one speaker per language, so these are pitch variations of it. |
| Speech speed | 100 | In percent, from 50 to 200. |
| Volume | 50 | In percent. |
| Repeat an unacknowledged alarm every | 30 | In seconds. 0 speaks each alarm once. |
| Snooze time | 5 | In minutes. |

Changes apply to the next spoken message; no restart is needed.

The language selects the voice. It does not translate: a Signal K notification written in English is read with Italian pronunciation if the language is `it-IT`. Sentences the device composes itself, which are the startup announcement and the NMEA 2000 alerts, are in the chosen language.

## RGB LED

The LED shows the most severe alarm currently raised.

| LED | Meaning |
|-----|---------|
| Off | No alarm |
| Blue | Alert (Signal K) or caution (NMEA 2000) |
| Yellow | Warning |
| Red | Alarm |
| Red, blinking fast | Emergency |

The LED **blinks** while at least one alarm is waiting for a reaction, and is **steady** once all alarms have been acknowledged or snoozed. It goes off when the alarm conditions have cleared.

The LED does not show the WiFi or Signal K connection status. Use the web interface's status page for that.

## Buttons

Both buttons act on all the alarms that are waiting at that moment.

| Button | Effect |
|--------|--------|
| Acknowledge | The alarms are no longer spoken. The LED stays lit, steady, until the conditions clear. |
| Snooze | The alarms keep quiet for the snooze time. If they are still raised afterwards, they are spoken again. |

An alarm that changes, for example from warning to alarm or with a new message, is announced again even if it had been acknowledged.

The BOOT button on the board keeps its SensESP function: a short press restarts the device, a long press resets the network settings and a very long press does a factory reset.

## How alarms are spoken

- The most severe alarm is spoken first.
- An alarm is repeated at the configured interval until it clears or is acknowledged or snoozed.
- Several alarms take turns, one sentence at a time.
