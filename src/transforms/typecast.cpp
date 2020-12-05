#include "transforms/typecast.h"
#include <math.h>

BoolToInt::BoolToInt() : Typecast<bool, int>([](bool in) -> int { return in ? 1 : 0; })
{
}

IntToBool::IntToBool() : Typecast<int, bool>([](int in) -> bool { return in != 0; })
{
}

IntToFloat::IntToFloat() : Typecast<int, float>([](int in) -> float { return (float)in; })
{
}

FloatToInt::FloatToInt() : Typecast<float, int>([](float in) -> int { return (int)in; })
{
}


RoundToInt::RoundToInt() : Typecast<float, int>([](float in) -> int { return (int)round(in); })
{
}
