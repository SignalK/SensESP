#include "nullable.h"

#include <cstdint>
#include <limits>

namespace sensesp {

// Invalid values below are set equal to NMEA 2000 "missing data" values

template <typename T>
T Nullable<T>::invalid_value_ = T{};
template <>
float Nullable<float>::invalid_value_ = -1e9;
template <>
double Nullable<double>::invalid_value_ = -1e9;
template <>
char Nullable<char>::invalid_value_ = 0xff;
template <>
uint8_t Nullable<uint8_t>::invalid_value_ = 0xff;
template <>
int8_t Nullable<int8_t>::invalid_value_ = 0x7f;
template <>
uint16_t Nullable<uint16_t>::invalid_value_ = 0xffff;
template <>
int16_t Nullable<int16_t>::invalid_value_ = 0x7fff;
template <>
uint32_t Nullable<uint32_t>::invalid_value_ = 0xffffffff;
template <>
int32_t Nullable<int32_t>::invalid_value_ = 0x7fffffff;
template <>
int Nullable<int>::invalid_value_ = 0x7fffffff;
template <>
uint64_t Nullable<uint64_t>::invalid_value_ = 0xffffffffffffffffLL;
template <>
int64_t Nullable<int64_t>::invalid_value_ = 0x7fffffffffffffffLL;

template <>
bool Nullable<bool>::invalid_value_ = false;


}  // namespace sensesp
