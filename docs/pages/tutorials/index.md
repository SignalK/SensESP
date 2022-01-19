---
layout: default
title: Tutorials
nav_order: 90
---

# Tutorials

This page lists the tutorials available on the SensESP website.
The tutorials are down-to-earth guides that teach you how to achieve specific goals with SensESP.
They can vary a lot in scope and complexity, and often also discuss not just SensESP itself but also how to integrate SensESP with external hardware and software components.

The following labels are used to categorize the tutorials:

| Label | Description |
| :---: | :--- |
| &#x1f4cc; | Essential: Concepts important for all levels of users
| &#x1F424; | Newbie: Slow-paced tutorials suitable for people with little programming experience |
| &#x1F47D; | Advanced: Intermediate or advanced content that highlight quirks or interesting techniques |

## Interfacing with Software Libraries and Hardware

### &#x1f4cc; [Using `RepeatSensor` to Interface a Hardware Sensor Library: BMP280](bmp280)

There are literally hundreds of sensor libraries that are compatible with the ESP32 and the Arduino Framework. The vast majority of them are read following the same pattern: initiate the hardware, do any required configuration, then call a specific function to read a specific value. SensESP interfaces with these types of sensors very easily, as illustrated in this Tutorial, which shows how to read temperature and barometric pressure with an Adafruit-compatible BMP280.

### &#x1f4cc; [Using `RepeatSensor` with ANY External Hardware Sensor Library](bmp280_part_2)

This tutorial illustrates how to integrate an arbitrary Arduino Framework compatible sensor library with SensESP.

## SensESP Concepts

### &#x1F424; &#x1f4cc; [Implementing Arbitrary Transforms with `LambdaTransform`](lambda_transform)

This tutorial shows how to define custom transforms using the `LambdaTransform` class.

### &#x1F424; [Using `AnalogInput` to Calculate a Tank Level](tank_level)

Confused about how to calculate a tank level from a single analog input?
This tutorial shows how to do it.
