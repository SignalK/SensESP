### Example: fuel level sensor

Example of a SensESP fuel level sensor implementation. Designed to be used with a 0-180 (VDO) type (linear resistance) transducer, as pictured below.

![Transducer](https://raw.githubusercontent.com/signalk/SensESP/master/examples/fuel_level_sensor/sensor.jpg "Linear Transducer")

The circuit required to measure the resistance is based on this: [voltage devider calculator](http://www.ohmslawcalculator.com/voltage-divider-calculator); basically you connect 1 transducer wire to ground (GND), another one to A0 (SIGNAL) and put a resistor between the 3.3v rail and the SIGNAL wire (in series with A0). The circuit works because the resistor is a known value, hence we can deduce the value of the transducer.

To use this code, you need to perform a measurement of your min/max values once and adjust the `scale` factor accordingly. See `main.cpp` in this folder for details.

![Circuit](https://raw.githubusercontent.com/signalk/SensESP/master/examples/fuel_level_sensor/circuit.png "DIY circuit, including 9-30 -> 5 volt DC/DC voltage regulator, resistor and shoddy soldering")