---
layout: default
title: Tank level calculations
parent: How do I ... ?
nav_order: 20
---
# Using AnalogInput to Calculate a Tank Level

After many, many exchanges with users about using SensESP to send a tank level to Signal K, it's obvious that it's a complex topic. Hopefully, this page will make it easy to understand - or at least make it possible to work. There are four distinct examples - make sure that the description of the example fits your situation.

When you use an Analog Input pin on any microcontroller, the output is going to be an integer from 0 to some number. On an ESP8266, it's 0 - 1023; on an ESP32, it's 0 - 4095. SensESP's [`AnalogInput` Sensor](https://signalk.org/SensESP/generated/docs/class_analog_input.html) compensates for that difference by providing a parameter called `output_scale`, which takes the raw value from the `AnalogRead()` of the pin and maps it to the correct point on a scale of 0 to `output_scale - 1`. The default value for `output_scale` is 1024, but if you're using an ESP32 and you want to get the full precision, you can make it 4096. However, both 1024 and 4096 are usually not meaningful numbers to us humans - we like things expressed in percentages, or ratios, or sometimes flushes, as you'll see below. Here, the use of the AnalogInput Sensor (and probably the Linear Transform) will be explained in hopefully simple terms, to help you get an appropriate number out of an AnalogIn pin and into Signal K.

For all of these examples, we'll use a tank level sensor that reads the level of liquid in a tank and sends the value to the ESP's AnalogIn pin.

## EXAMPLE 1: a tank level sensor that outputs 0 when the tank is empty and 1023 when the tank is full.
You, being human, would rather see empty and full expressed as a percentage from 0 to 100. When you instantiate your AnalogInput Sensor, do it with an `output_scale` of 100, like this:
```
// assume `pin`, read_delay`, and `config_path` are already defined in main.cpp
auto* tank_level = new AnalogInput(pin, read_delay, config_path, 100);
```
AnalogInput will convert any value from empty (0) to full (1023) into the proper percentage from empty (0) to full (100). Ta da!

However... tank levels are *supposed* to be sent to Signal K as a "ratio" - a number between 0 and 1, where 0 is empty and 1 is full. So, instead of using 100 for the `output_scale`, use 1 to express the value as a ratio.

## EXAMPLE 2: a sensor that outputs 0 when the tank is empty, but something less than 1023 when the tank is full
It will be very unusual for your sensor to output its full scale value (1023 in our first example) when the tank is full. Far more likely is that it will output a value less than its full scale value - let's say it outputs 826 when the tank is full. To keep this relatively simple, we'll still assume the output is 0 when the tank is empty.

- empty_value = 0
- full_value = 826
- range = full_value minus empty_value = 826 - 0 = 826

We still want to express our final output as a percentage of full, so now, a value of 826 should be output as 100 (percent). A little algebra:

- 1024 / 826 = X / 100
- 1.239709443 = X / 100
- 1.239709443 x 100 = X / 100 x 100
- 123.9709443 = X (round to 124)

So if you set `output_scale` to 124, a reading of 826 will be 100 on that scale.
The simple formula for you to use to calculate your output_scale is:

- output_scale = 1024 / range x 100
- or even simpler:
- output_scale = 102400 / range (102400 / 826 = 123.9709443, which you can round to 124)
```c++
auto* tank_level = new AnalogInput(pin, read_delay, config_path, 124);
```
will convert any value from 0 - 826 into the appropriate percentage, 0 - 100.

However... tank levels are *supposed* to be sent to Signal K as a "ratio" - a number between 0 and 1, where 0 is empty and 1 is full. So, instead of using 124 for the `output_scale`, you need to divide it by 100: 124 / 100 = 1.24.

## EXAMPLE 3: a sensor that outputs something other than 0 when the tank is empty
Not only is it unlikely that your sensor will output 1023 when the tank is full, it's also unlikely to output 0 when the tank is empty. It's far more likely to output some small-ish value, like 43. When that happens, we're going to have to do two things to get our 0 - 100% scaled output: we have to get the proper scale, and we also have to have an `offset`, to compensate for the fact that empty = 43.

There are a couple ways to do this, but your humble author believes the simplest way is to let AnalogInput provide whatever raw value it's going to provide with its default `output_scale` of 1024, and then run that raw value through a [`Linear`](https://signalk.org/SensESP/generated/docs/class_linear.html) Transform, using the transform's `multiplier` and `offset` parameters. Here's how it works (to avoid confusion with previous examples, note that the example numbers are totally different):

In your `main.cpp` start by using only AnalogInput connected to SKOutputNumber:
```c++
// notice that `output_scale` is not provided, so the default of 1024 is used
auto* tank_level = new AnalogInput(pin, read_delay, analog_in_config_path);
tank_level->connect_to(new SKOutputNumber(sk_path)); // assumes sk_path has already been defined
```
Get these two values for your tank: the value when the tank is empty, and the value when the tank is full (with examples):

- empty_value (43)
- full_value (991)
- range = full_value minus empty_value (991 - 43 = 948)

We want to spread 948 equally across the percentage range of 0 - 100, so:

- range divided by 100 = "output units per percent" (948 / 100 = 9.48)

In other words, each percentage point represents 9.48 "output units" from the AnalogIn pin. So if we take whatever the output is from AnalogIn and divide it by 9.48, we'll get the appropriate percentage:

- empty tank: 43 / 9.48 = 4.536
- half full tank: (43 + 474) (which is 1/2 the range) / 9.48 = 54.536
- full tank: 991 / 9.48 = 104.536

Obviously, every value is 4.536 bigger than it should be, so we know that our `offset` parameter needs to be `-4.536`.

Above, we're dividing our raw value by 9.48, but `multiplier` is a multiplier, not a divisor. That is, the math is `raw_value X multiplier`, not `raw_value / multiplier`. We can convert a divisor into a multiplier simply:

multiplier = 1 / divisor (1 / 9.48 = 0.1055), and prove it with the same three sample numbers above:

- 43 x 0.1055 = 4.536: add the `offset` (-4.536), and you get 0%.
- (43 + 474) x 0.1055 = 54.546: add the `offset` (-4.536), and you get 50%.
- 991 x 0.1055 = 104.551: add the `offset` (-4.536), and you get 100%.

So, by making Linear's `multiplier` = 0.1055 and `offset` = -4.536, it will convert any value from  AnalogInput into a percentage from 0 - 100.

Here are your important numbers and formulas for this example (with example numbers):
- empty_value (43)
- full_value (991)
- range = full_value - empty_value (991 - 43 = 948)
- divisor = range / 100 (948 / 100 = 9.48)
- multiplier = 1 / divisor (1 / 9.48 = 0.1055)
- offset = 100 - full_value x multiplier (100 - (991 x 0.1055) = -4.536)

Now, you add `Linear` to your `main.cpp` with your calculated `multiplier` and `offset` values:
```
auto* tank_level = new AnalogInput(pin, read_delay, analog_in_config_path);
tank_level->connect_to(new Linear(0.1055, -4.536))
          ->connect_to(new SKOutputNumber(sk_path));
```
However... tank levels are *supposed* to be sent to Signal K as a "ratio" - a number between 0 and 1, where 0 is empty and 1 is full. So, instead of using 0.1055 for the `multiplier`, you need to divide it by 100: 0.1055 / 100 = 0.001055. And the `offset` needs to be divided by 100, too: -4.536 / 100 = -0.04536.


## ADVANCED EXAMPLE: Outputting something other than a percentage or a ratio (number of "flushes" for a blackwater tank)
You may not always want to see your output as a percentage. In the case of a blackwater tank, it might be more useful to express the level in terms of "flushes", so that you'll know how many more times you can flush the head before you have to pump out. (This is how the author does it, and trust the author - it works really well!) The only difference in this example is that instead of using 100 (percent) in the formulas, you use the number of flushes that represents a full tank. This will be only an estimate, as each flush isn't exactly the same volume as every other flush, but it's close enough. Here's how you do it:

In your `main.cpp` start by using only AnalogInput connected to SKOutputNumber:
```c++
// notice that `output_scale` is not provided, so the default of 1024 is used
auto* tank_level = new AnalogInput(pin, read_delay, analog_in_config_path);
tank_level->connect_to(new SKOutputNumber(sk_path)); // assumes sk_path has already been defined
```
Get these two values for your tank: the value when the tank is empty, and the value when the tank is full (with example numers):

- empty_value (43)
- full_value (991)
- range = full_value minus empty_value (991 - 43 = 948)

In addition, you need to know the value of an average flush:
- with the tank not close to being full, record the value: (354)
- flush the head 3 times, and record the value: (417)
- calculate the average per flush: ((417 - 354) / 3 = 21)

Now you know that your range is 948 and an average flush is 21, so you have about 45 flushes in a full tank. (948 / 21 = 45)

Here are the important values and formulas, with our example numbers:
- empty_value (43)
- full_value (991)
- range (991 - 43 = 948)
- avg_flush (21)
- total_flushes (45)
- multiplier = 1 / avg_flush (1 / 21 = 0.0476)
- offset = total_flushes - (full_value x multiplier)  (45 - (991 x 0.0476) = 45 - 47.1716 = -2.1716)

Now, you add `Linear` to your `main.cpp` with your calculated `multiplier` and `offset` values:
```c++
auto* tank_level = new AnalogInput(pin, read_delay, analog_in_config_path);
tank_level->connect_to(new Linear(0.0476, -2.1716))
          ->connect_to(new SKOutputNumber(sk_path));
```
Note that this approach doesn't actually "count" flushes; instead, it estimates the number of remaining flushes until full. As noted above, it works great! And in this case, we do NOT want to send the value to Signal K as a ratio, as we did in the other examples, so these calculated values for `multiplier` and `offset` are the ones to use.
