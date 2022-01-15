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


Each of the Tutorials is briefly described below.

## Minimal Example of Sending Data to Signal K

This should be the very first Tutorial you go through as a new SensESP user. It will introduce you to the basic structure of every SensESP project, and explain all of the "boilerplate" code that's in every project. Subsequent tutorials won't do this, so you need to start with this one.

It demonstrates how to send a single bit of data (the status of GPIO13) to a Signal K Server, so you can try it out with only an ESP32 and a jumper wire.

[Click here to see the Tutorial](minimal_sk).

## Sending Data from Two Sensors to Signal K

This Tutorial shows how to read more than one physical sensor in a single SensESP project. [Click here to see it](minimal_sk_two_sensors).

## Reading a SensESP-specific External Sensor

Many sensors that you might want to use are read with a library specific to that sensor. Many of these can be read with a very generic approach, illustrated in the [next tutorial](bmp280). But some require a little more code to get them to work easily with SensESP, so they use an additional "helper" library, several of which you can find [here](https://github.com/SensESP).

This Tutorial shows how to use one of these, the very popular 1-Wire temperature sensor. [Click here to see it](one_wire).

## Reading a "Generic" External Sensor, Part 1

There are literally hundreds of sensors that can be used by Arduino-compatible microprocessors like the ESP32. The vast majority of them are read following the same pattern: initiate the hardware, do any required configuration, then call a specific function to read a specific value. SensESP interfaces with these types of sensors very easily, as illustrated in this Tutorial, which shows how to read temperature and barometric pressure with an Adafruit-compatible BMP280. [Click here to see it](bmp280).

## Reading a "Generic" External Sensor - Part 2

This Tutorial builds on Part 1, explaining how to use that approach for almost any Arduino-compatible sensor. [Click here to see it](bmp280_part_2).

## Reading a "Generic" External Sensor - Part 3

This Tutorial shows how to read more than one value from the same physical sensor. [Click here to see it](ina219_2_values).

## Reading Multiple Values from Multiple Sensors

This Tutorial combines everything in the previous Tutorials, showing you how to read multiple values from multiple sensors in the same SensESP project. [Click here to see it](multiple_sensors).
