#ifndef _typecast_H_
#define _typecast_H_

#include <functional>

#include "transforms/lambda_transform.h"

/**
 * Typecast is a transform that allows you to convert from one
 * data type to another. To use Typecast, simply construct
 * a new instance of Typecast.
 * <p>If the data type of IN is capable of doing
 * a typecast to OUT on its own (i.e. C++ has an implicit
 * conversion available, or IN defines a overloaded
 * typecast operator for OUT), then no additional
 * work is needed.  If a compiler derived typecast from
 * IN to OUT does not exist (or is not the functionality
 * you are looking for), then you must pass in a
 * lambda expression that is capable of doing the
 * conversion explicitly.
 * @tparam The data type the typecast accepts as input
 * @tparam The data type the typecast outputs after conversion
 */
template <typename IN, typename OUT>
class Typecast : public LambdaTransform<IN, OUT>
{

public:
    Typecast(OUT (*cast)(IN input) =
       [](IN input) -> OUT { return (OUT)input; } ) : LambdaTransform<IN, OUT>(cast) {}
};


typedef Typecast<int, bool> IntToBool;
typedef Typecast<bool, int> BoolToInt;

typedef Typecast<int, float> IntToFloat;
typedef Typecast<float, int> FloatToInt;

/**
 * A Typecast transform that takes as its
 * input a float number, rounds it to
 * the nearest whole number, then outputs
 * it as an int. 
 */
class RoundToInt : public Typecast<float, int>
{

public:
    RoundToInt();
};

#endif
