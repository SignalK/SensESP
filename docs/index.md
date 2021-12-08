---
layout: default
title: Home
nav_order: 1
description: "SensESP is a universal sensor development framework for the ESP32 platform."
permalink: /
---
# Introduction

SensESP is a Signal K sensor development library for the ESP32 platform.
It can be used as a high-level toolkit for creating ESP32-based hardware sensoring devices.
Originating from the [Signal K](https://signalk.org) ecosystem, supports the marine Signal K protocol out of the box, but also provides facilities for creating generic sensor devices.

SensESP can be used to build sensor devices that report the sensed values back to a Signal K server, or to react to commands delivered by the server.
With suitable hardware (see for example [SH-ESP32](https://hatlabs.github.io/sh-esp32)), it is also possible to interface SensESP with an NMEA 2000 network.

SensESP supports a wide range of common sensor hardware with a set of [add-on libraries](pages/additional_resources/add-ons/), but even if direct support for a particular device is missing, using generic Arduino framework libraries is still possible.

## Target audience

SensESP is intended for developers who want to create sensor devices for the ESP32 platform.
This includes beginners and seasoned veterans alike.
The library is designed to be easy to use, and as a project we strive to be beginner-friendly.
However, it is still a software development library, and to use it, you are expected to be able to use basic text editors.
