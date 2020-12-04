#ifndef _typecast_H_
#define _typecast_H_

#include <functional>

#include "transforms/transform.h"

/**
 * Typecast is a transform that allows you to convert from one
 * data type to another. To use Typecast, simply constructor
 * a new instance, passing in a lambda function that takes
 * as its input a value of type "IN" and returns the
 * value as an "OUT".
 * <p>The two most common typecasts one might use in
 * SensESP (bool to int, and int to bool) have been pre-defined
 * for easier user.
 * @see IntToBool
 * @see BoolToInt
 */
template <typename IN, typename OUT>
class Typecast : public Transform<IN, OUT>
{

public:
    Typecast(std::function<OUT(IN)> cast) : Transform<IN, OUT>(""), cast_{cast} {}

    virtual void set_input(IN new_value, uint8_t input_channel = 0) override
    {
        this->emit(cast_(new_value));
    }

private:
    std::function<OUT(IN)> cast_;
};

/**
 * A Typecast transform that takes as its
 * input an integer number, and outputs
 * booean TRUE if the number is non-zero.
 */
class IntToBool : public Typecast<int, bool>
{

public:
    IntToBool();
};

/**
 * A Typecast transform that takesa as its
 * input a boolean value, and outputs "1"
 * if it is true, and "0" if it is false.
 */
class BoolToInt : public Typecast<bool, int>
{
public:
    BoolToInt();
};

#endif
