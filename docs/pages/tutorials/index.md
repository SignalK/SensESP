---
layout: default
title: Tutorials
nav_order: 42
has_children: true
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

### &#x1f4cc; [Using External Sensor Libraries: BMP280](bmp280)

There are literally hundreds of sensor libraries that are compatible with the ESP32 and the Arduino Framework. The vast majority of them are read following the same pattern: initiate the hardware, do any required configuration, then call a specific function to read a specific value. SensESP interfaces with these types of sensors very easily, as illustrated in this Tutorial, which shows how to read temperature and barometric pressure with an Adafruit-compatible BMP280.

### &#x1f4cc; [Using Any External Sensor Libraries](bmp280_part_2)

This tutorial illustrates how to integrate an arbitrary Arduino Framework compatible sensor library with SensESP.

### &#x1F424; [Using a SensESP-specific External Sensor Library](one_wire)

Many sensors that you might want to use are read with a library specific to that sensor. Many of these can be read with a very generic approach, illustrated in the [BMP280 Tutorial](bmp280). But some require a little more code to get them to work easily with SensESP, so they use an additional "helper" library, several of which you can find [here](https://github.com/SensESP).

This Tutorial shows how to use one of these, the very popular 1-Wire temperature sensor.

### &#x1F424; [Reading Multiple Values from an External Sensor Library](ina219_2_values)

This tutorial shows how to read more than one value from the same sensor library.

### &#x1F424; [Reading Multiple Values from Multiple Sensors Libraries](multiple_sensors)

This tutorial shows you how to read multiple values from multiple sensors in the same SensESP project.

## Signal K

### &#x1F424; [Sending Data to Signal K](minimal_sk)

This tutorial will introduce you to the basic structure of a typical SensESP project, and explain all of the "boilerplate" code that's in every project.

It demonstrates how to send a single bit of data (the status of GPIO13) to a Signal K Server, so you can try it out with only an ESP32 and a jumper wire.

### &#x1F424; [Sending Data from Two Sensors to Signal K](minimal_sk_2sensors)

This Tutorial shows how to read more than one physical sensor in a single SensESP project.
