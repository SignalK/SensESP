#ifndef SENSESP_TRANSFORMS_TYPECAST_H_
#define SENSESP_TRANSFORMS_TYPECAST_H_

#include <functional>

#include "lambda_transform.h"

namespace sensesp {

/**
 * @brief Converts input from one
 * data type to another, then outputs the new type.
 *
 * To use Typecast, simply construct
 * a new instance of Typecast.
 *
 * <p>If the data type of IN is capable of doing
 * a typecast to OUT on its own (i.e. C++ has an implicit
 * conversion available, or IN defines a overloaded
 * typecast operator for OUT), then no additional
 * work is needed.
 *
 * If a compiler-derived typecast from
 * IN to OUT does not exist (or is not the functionality
 * you are looking for), then you must pass in a
 * lambda expression that is capable of doing the
 * conversion explicitly.
 *
 * @tparam The data type of input.
 *
 * @tparam The data type of output.
 */
template <typename IN, typename OUT>
class Typecast : public LambdaTransform<IN, OUT> {
 public:
  Typecast(std::function<OUT(IN)> cast = [](IN input) -> OUT {
    return static_cast<OUT>(input);
  })
      : LambdaTransform<IN, OUT>(cast) {}
};

typedef Typecast<int, bool> IntToBool;
typedef Typecast<bool, int> BoolToInt;

typedef Typecast<int, float> IntToFloat;
typedef Typecast<float, int> FloatToInt;

/**
 * @brief Takes as its
 * input a float, rounds it to
 * the nearest whole number, then outputs
 * it as an int.
 */
class RoundToInt : public Typecast<float, int> {
 public:
  RoundToInt();
};

}  // namespace sensesp
#endif
