#include "typecast.h"

#include <math.h>

namespace sensesp {

RoundToInt::RoundToInt()
    : Typecast<float, int>([](float in) -> int { return (int)round(in); }) {}

}  // namespace sensesp
