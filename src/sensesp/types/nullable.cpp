#include "nullable.h"

#include <limits>
#include <cstdint>

namespace sensesp {

template <typename T> T Nullable<T>::invalid_value_ = T{};
template <> int Nullable<int>::invalid_value_ = std::numeric_limits<int>::lowest();
template <> float Nullable<float>::invalid_value_ = std::numeric_limits<float>::lowest();
template <> double Nullable<double>::invalid_value_ = std::numeric_limits<double>::lowest();
template <> char Nullable<char>::invalid_value_ = 255;

}  // namespace sensesp
