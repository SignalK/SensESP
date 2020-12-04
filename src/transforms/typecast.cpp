#include "transforms/typecast.h"

BoolToInt::BoolToInt() : Typecast<bool, int>([](bool in) -> int { return in ? 1 : 0; })
{
}

IntToBool::IntToBool() : Typecast<int, bool>([](int in) -> bool { return in != 0; })
{
}
