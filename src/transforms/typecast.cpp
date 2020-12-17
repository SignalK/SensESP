#include "transforms/typecast.h"
#include <math.h>

RoundToInt::RoundToInt() : Typecast<float, int>([](float in) -> int { return (int)round(in); })
{
}
