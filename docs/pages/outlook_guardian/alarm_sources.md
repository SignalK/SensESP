---
layout: default
title: Alarm sources
parent: Outlook Guardian
nav_order: 40
---

# Alarm sources

The device has two sources of alarms and uses one at a time:

- While the websocket connection to the **Signal K** server is up, only Signal K notifications are used and the NMEA 2000 bus is ignored.
- While it is down, the **NMEA 2000** alerts are used.

A Signal K server that is itself connected to the NMEA 2000 bus already turns the alerts into notifications, so this avoids announcing everything twice.

When the source changes, the alarms of the other source are dropped, as they are no longer kept up to date. During the few seconds after power-up before the Signal K connection is established, an NMEA 2000 alert may be announced and then announced again from Signal K.

## Signal K notifications

The device subscribes to `notifications.*` of the vessel. A notification value looks like this:

```json
{
  "state": "alarm",
  "method": ["visual", "sound"],
  "message": "Engine coolant temperature is high"
}
```

### State

| `state` | Treated as | LED |
|---------|-----------|-----|
| `emergency` | Emergency | Red, fast blink |
| `alarm` | Alarm | Red |
| `warn` | Warning | Yellow |
| `alert` | Alert | Blue |
| `normal`, `nominal`, or a null value | Cleared | Off |

### Message

The `message` text is spoken as it is. A notification without a message is shown on the LED but not spoken.

### Method

`method` tells displays how to indicate the alarm, and is how devices on a Signal K network tell each other that somebody has reacted:

| `method` | Meaning for the device |
|----------|-----------------------|
| Contains `sound`, or is missing | Spoken |
| Does not contain `sound` | Silenced somewhere else: not spoken, LED on |
| Empty list | Acknowledged somewhere else: not spoken, LED steady |

The `status.silenced` and `status.acknowledged` fields reported by servers with the Notifications API are honoured in the same way.

### What the buttons send

A button press sends a PUT request over the websocket for each affected notification, on the path `<notification path>.method`:

| Button | Value sent |
|--------|-----------|
| Acknowledge | `[]` |
| Snooze | `["visual"]`, and when the snooze time is over, the value the notification had before |

For example, acknowledging `notifications.propulsion.port.temperature` sends:

```json
{
  "context": "vessels.self",
  "requestId": "…",
  "put": {
    "path": "notifications.propulsion.port.temperature.method",
    "value": []
  }
}
```

The Signal K server handles these requests itself; no plugin is needed. The device must have been granted read/write access. A refused request is logged on the serial console.

## NMEA 2000 alerts

The device listens for two messages:

| PGN | Name | Used for |
|-----|------|----------|
| 126983 | Alert | The type, category and state of an alert |
| 126985 | Alert Text | Its description and location |

### Composing the sentence

The two messages arrive separately. The device waits up to 2 seconds for the text of a newly raised alert, then speaks a sentence made of the category, the type, the description and the location:

> Technical warning. Low oil pressure. Engine room.

If no text arrives in time, the alert number is used instead:

> Technical warning number 12.

If the text arrives later, the complete sentence is spoken then.

The category and type words are in the configured language. The description and location are spoken as sent by the device that raised the alert.

### Alert type

| Alert type | Treated as | LED |
|------------|-----------|-----|
| Emergency alarm | Emergency | Red, fast blink |
| Alarm | Alarm | Red |
| Warning | Warning | Yellow |
| Caution | Alert | Blue |

### Alert state

| Alert state | Effect |
|-------------|--------|
| Active, Awaiting acknowledge | Raised and spoken |
| Silenced | Raised, not spoken |
| Acknowledged | Raised, treated as acknowledged |
| Normal, Disabled | Cleared |

### Buttons

The device is a listen-only node, so on NMEA 2000 the acknowledge and snooze buttons act on this device only. Nothing is sent to the equipment that raised the alert.
