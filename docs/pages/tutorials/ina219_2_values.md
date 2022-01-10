---
layout: default
title: Reading Multiple Values from a Generic External Sensor
parent: Tutorials
nav_order: 22
---

# Tutorial: Reading Multiple Values from a Generic External Sensor

[The previous Tutorial](../bmp280_part_2) illustrated how to use a very generic approach, using a SensESP Sensor called a *RepeatSensor*, to read a value from almost any Arduino-compatible sensor. This Tutorial will build on that, explaining how to read more than one value from the same physical sensor. You need to go through that previous Tutorial first, and modify the `main.cpp` as illustrated there, because that will be the starting point for this Tutorial.

The INA219 can read four values:

* Shunt Voltage (in mV)
* Bus Voltage (in V)
* Current (in mA)
* Power (in mW)

We used Current in the previous Tutorial. This tutorial will show you how to ALSO read Bus Voltage, from the same physical sensor, in the same SensESP project.

This same approach will work with any other physical sensor that can read more than one value. It's up to you to determine what all of its values are by reading the documentation for the sensor's library.

## platformio.ini

You don't need to make any changes at all to `platformio.ini`. The reason should be obvious - the library to read the first value is already there, and the same library is used to read all the other values, too.

## main.cpp

All of the lines we modified in the last Tutorial to read Current from an INA219 (instead of Temperature from a BMP280) are listed below, without explanation. (See the previous Tutorial if you need an explanation.) The new lines for reading Bus Voltage from the same INA219 are identified with a comment starting with "NEW".

```c++
// Sensor-specific #includes:
#include <Adafruit_INA219.h>
#include <Wire.h>
```

No change necessary to the above two lines, which should be obvious to you.

```c++
Adafruit_INA219 ina219;
float read_current_callback() { return (ina219.getCurrent_mA() / 1000); }
// NEW: define the function to read the Bus Voltage:
float read_bus_volts_callback() { return ina219.getBusVoltage_V(); }
```

Above, you still create `ina219` only once, then you use it in the Current callback, and the new Bus Volts callback. It's up to you to read the sensor library's documentation to determine which function reads the value you're interested in. Above, it's `getBusVoltage_V()`. And since it's in volts (not mV or some other version of volts), you don't need to do any math, since voltage values are sent to Signal K as volts.

```c++
// Initialize the INA219 using the default address
ina219.begin();
```

No change necessary to the above line: the same sensor instance is used to read whatever values we want to read, so it's initialized / started only once.

```c++
auto* house_bank_current =
      new RepeatSensor<float>(read_interval, read_current_callback);

const char* sk_path = "electrical.batteries.houseBank.current";
house_bank_current->connect_to(new SKOutputFloat(sk_path));

// NEW: create a pointer to a new RepeatSensor that will read the Bus Voltage:
auto* house_bank_bus_volts =
      new RepeatSensor<float>(read_interval, read_bus_volts_callback);
// NEW: send the value to Signal K as a Float, with the appropriate Path
house_bank_bus_volts
     ->connect_to(new SKOutputFloat("electrical.batteries.houseBank.volts"));     
```
Note that you explicitly select the data type of `RepeatSensor` - in this example, it's `RepeatSensor<float>`. If `getBusVoltage_V()` returned an integer, you would use `RepeatSensor<int>`.

The rest of `main.cpp` is exactly the same as in all of the other Tutorial examples.

## Summary

* You can read multiple values from the same physical sensor by creating a separate instance of RepeatSensor for each value you want to read, and then sending the value from each instance to Signal K with an appropriate Path.

## Related Tutorials

* The Tutorial that this one is based on: [Using ANY Generic External Sensor Library - INA219](../bmp280_part_2).
* [Using Multiple Sensors in a Single SensESP Project](../multiple_sensors).
