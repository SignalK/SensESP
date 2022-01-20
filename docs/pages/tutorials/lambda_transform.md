---
layout: default
title: Implementing Arbitrary Transforms
parent: Tutorials
---
# Implementing Arbitrary Transforms with `LambdaTransform`

In SensESP, a Transform is a class that takes in a value, does something to it, and then outputs the result, the "transformed" value. At least one Transform, a specialized Transform called SKOutput, is used with virtually every sensor you connect to the ESP. (It takes the final value from the sensor and sends it to the Signal K Server.) [Click here](https://github.com/SignalK/SensESP/wiki/SensESP-Overview-and-Programming-Details#transforms) for more information about Transforms.

Originally, each Transform was written as its own separate Class, with its own .h and .cpp file, and its own set of methods for configuring any configurable parameters (get_configuration(), set_configuration(), get_config_schema(), and the SCHEMA itself). For Transforms with complex algorithms doing the transforming of the input into output, this approach is still necessary. However, for simpler Transforms, in which the "transformation" is just a few simple lines of code (often only a single line), something more "quick and easy" was wanted: the LambdaTransform.

The LambdaTransform gets its name from "lambda expressions", a programming concept for creating functions when and where you need them. Another name for lambda expressions is "anonymous functions": they're typically created on the spot and passed to some other function that will then call them. There are other exciting related concepts such as closures that have been shoehorned into C++ lambda expressions, making the C++ syntax a bit more confusing than in most other programming languages. If you're interested in learning more about lambda expressions in C++, this [RIP Tutorial](https://riptutorial.com/cplusplus/example/1854/what-is-a-lambda-expression-) does an excellent job of explaining them. Read as much of that as you like, then return here for this tutorial, which will walk you through creating and using a lambda expression in several LambdaTransform examples.

In the LambdaTransform context, it is actually safe to ignore all that "theory of computation" nonsense. Since the lambda functions are assigned to a variable, they're not even that anonymous! They're just functions defined with an alternative syntax. Actually, for creating Transforms, you could use plain old ordinary functions, too! (In that case you'd just have to define them at a different location in the file because C++ doesn't support nested functions.)

If you're already familiar with lambda expressions, and/or if you're a relatively experienced C++ developer, you may be able to learn all you need to learn about the LambdaTransform by looking at the [lambda_transform header file](https://github.com/SignalK/SensESP/blob/master/src/transforms/lambda_transform.h) and the [lambda_transform.cpp example](https://github.com/SignalK/SensESP/blob/master/examples/lambda_transform.cpp).

If this is all new to you, let's get started!

## Example #1: A Very Basic LambdaTransform
This example will introduce all the parts of the LambdaTransform, but will use the simplest implementation: a function that takes no parameters other than `input`, which is the value coming into the Transform to be transformed. It will take an `int` as input and transform it into a `bool`: 1 will be transformed into `true` and `0` will be transformed into `false`.

First, let's write the function that's going to do the work of this Transform, and describe the lambda expression syntax in the process.

```
auto int_to_bool_function = [](int input) ->bool {
     if (input == 1) {
       return true;
     }
     else { // input == 0
       return false;
     }
};
```
The basic format of a lambda expression is `[] () {}` - the Capture List, the Parameter List, and the Function Body, respectively. A slight variation on this basic format is to specify the return type of the function body, like this:
```
[] () ->returnType {}
```
which is the format we'll use in our first example because it adds a little clarity. Look at the code above:
- the Capture List is empty, and always is in the LambdaTransform.
- the Parameter List has one parameter: an `int` called `input`.
- the returnType is `bool`.
- the Function Body contains the transformation logic: the `if-else` code above.

Now we need to instantiate a LambdaTransform object that uses our `int_to_bool_function`.
```
auto int_to_bool_transform = new LambdaTransform<int, bool>(int_to_bool_function);
```
- `auto` is the return type of this transform and will be determined by the compiler; it will be whatever is returned by our `int_to_bool_function`. (So, it will be a `bool`.)
- `int_to_bool_transform` is the name of the LambdaTransform object. (Actually, it's a pointer to the object, since it's created by `new`.)
- `LambdaTransform<int, bool>(int_to_bool_function)` does two things: it says that our Transform is going to have an `int` as input and a `bool` as output (with the `LambdaTransform<int, bool>` part); and it says the function that we want to call whenever a value is passed to this Transform is our `int_to_bool_funtion`.

Finally, we use it to transform the integer output of a DigitalInputValue Sensor into `true` or `false` , then send it to Signal K:
```
auto* digital_input = new DigitalInputValue(pin, INPUT, CHANGE);

digital_input->connect_to(int_to_bool_transform)
             ->connect_to(new SKOutputBool("some.sk.path"));
```
In SensESP, all of the code we've written so far would all be together inside `main.cpp`, after the SensESPApp is created, and before it's enabled. This is the "meat" of the program - the part that does all the gathering and processing and sending of data to the Signal K Server.

## Example #2: A Basic LambdaTransform with One Parameter
This example is going to get a bit more complex, but not much: we're just going to add a parameter to the function that does the transformation.
```
auto float_to_bool_function = [](float input, int threshold) -> bool {
     if (input >= threshold) {
       return true;
     }
     else {  // input < threshold
       return false;
     }
  };
```
The differences in this function, compared to our first example, are:
- The input type is `float`.
- There's a new parameter: an `int` called `threshold`.
- The new paramter is used in the function body, to help decide what to output.

Another difference in this example is that, now that we have a parameter other than `input`, we have to describe it. (Every LambdaTransform has `input` as a parameter, and since it comes from a Sensor or another Tranform, it doesn't have to be described.) Each additional parameter is described in a `struct` called `ParamInfo`, and it includes the parameter name ("threshold" in this example), and a parameter label that will be displayed in the Config UI ("Threshold" in this example). It's declared / defined in this example like this:
```
const ParamInfo* param_data = new ParamInfo[1]{
      {"threshold", "Threshold"}
};
```
Now, when we instantiate our LambdaTransform object, it looks like this:
```
auto float_to_bool_transform = new LambdaTransform<float, bool, int>
         (float_to_bool_function, 298, param_data);
```

Notice these differences from the first example:
- The LambdaTransform template now has three types instead of two: the input type `float`, the output type `bool`, and the type of the first parameter to our transform function, `int`.
- The parameter list for the function itself now has three parameters instead of just one: the name of our function, which we had in the first example; the `threshold` parameter (298); and the pointer to the array of ParamInfo (`param_data`).

Those are the only required parameters in this case, but if we want to make `threshold` configurable in the Config UI, we need to provide the configuration path for it, too, with one additional parameter, `"/coolantTemperature/threshold"`.
```
auto float_to_bool_transform = new LambdaTransform<float, bool, int>
         (float_to_bool_function, 298, param_data, "/coolantTemperature/threshold");
```
So now we have a Transform that takes in a `float`, outputs a `bool`, and uses an `int` in the logic of the transformation function. The function that's going to be called is our `float_to_bool_function`, with `298` as the `threshold` parameter used in that function. We have defined the configuration data for `threshold` in `param_data`, and finally, we've provided the path to get to that configuration data in the Config UI: `"/coolantTemperature/threshold".

Finally, the Transform will be used just like in the first example: the output of a sensor (`temp_sensor`) is connected to our LambdaTransform (`float_to_bool_transform`), the output of which is sent to Signal K as a bool:
```
temp_sensor->connect_to(float_to_bool_transform)
           ->connect_to(new SKOutputBool("some.sk.path"));
```
