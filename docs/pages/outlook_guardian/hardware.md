---
layout: default
title: Hardware
parent: Outlook Guardian
nav_order: 10
---

# Hardware

## Parts

| Part | Notes |
|------|-------|
| ESP32-S3-DevKitC-1 **N16R8** | 16 MB flash and 8 MB PSRAM are both required: the firmware is about 8.3 MB and the speech engine needs about 1.1 MB of PSRAM. |
| MAX98357A I2S amplifier breakout | Mono, 3 W class D. |
| Speaker | 4 or 8 Ω. |
| CAN transceiver | 3.3 V type such as SN65HVD230, for the NMEA 2000 bus. |
| 2 momentary push buttons | Normally open. |
| 12 V to 5 V DC-DC converter | To power the board from the boat or from the NMEA 2000 bus. |

## Pin assignments

The pins are set as build flags in the `[esp32s3]` section of `platformio.ini`. Change them there if your wiring differs.

| Function | Build flag | GPIO |
|----------|-----------|------|
| Amplifier LRC (word select) | `MAX98357A_LRC_PIN` | 4 |
| Amplifier BCLK | `MAX98357A_BCLK_PIN` | 5 |
| Amplifier DIN | `MAX98357A_DIN_PIN` | 6 |
| CAN transceiver TX (CTX / D) | `N2K_CAN_TX_PIN` | 17 |
| CAN transceiver RX (CRX / R) | `N2K_CAN_RX_PIN` | 18 |
| Acknowledge button | `ACK_BUTTON_PIN` | 15 |
| Snooze button | `SNOOZE_BUTTON_PIN` | 16 |
| On-board RGB LED | `PIN_RGB_LED` | 48 |
| On-board BOOT button (SensESP reset button) | `BUTTON_BUILTIN` | 0 |

The on-board RGB LED is on GPIO 48 on DevKitC-1 v1.0 boards and on GPIO 38 on v1.1 boards. Set `PIN_RGB_LED` accordingly.

## Wiring

### MAX98357A amplifier

| Amplifier pin | Connect to |
|---------------|-----------|
| VIN | 5 V |
| GND | GND |
| LRC | GPIO 4 |
| BCLK | GPIO 5 |
| DIN | GPIO 6 |
| SD, GAIN | Leave unconnected for the default mode and gain |
| + / − | Speaker |

### CAN transceiver

| Transceiver pin | Connect to |
|-----------------|-----------|
| 3V3 | 3.3 V |
| GND | GND |
| CTX / D | GPIO 17 |
| CRX / R | GPIO 18 |
| CANH | NMEA 2000 CAN-H (white wire) |
| CANL | NMEA 2000 CAN-L (blue wire) |

Connect to the bus with a drop cable on a T-piece. Do not add a termination resistor on the device: an NMEA 2000 backbone is already terminated at both ends. Many transceiver breakouts carry a 120 Ω resistor that has to be removed.

The device only listens on the bus. It does not claim an address and sends nothing.

### Buttons

Wire each button between its GPIO and GND. The internal pull-up resistors are enabled, so no external resistor is needed.

| Button | Connect between |
|--------|----------------|
| Acknowledge | GPIO 15 and GND |
| Snooze | GPIO 16 and GND |
