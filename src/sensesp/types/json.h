#ifndef SENSESP_SRC_SENSESP_TYPES_JSON_H_
#define SENSESP_SRC_SENSESP_TYPES_JSON_H_

#include <vector>

#include "ArduinoJson.h"

namespace ArduinoJson {
template <typename T>
struct Converter<std::vector<T> > {
  static void toJson(const std::vector<T>& src, JsonVariant dst) {
    JsonArray array = dst.to<JsonArray>();
    for (T item : src) array.add(item);
  }

  static std::vector<T> fromJson(JsonVariantConst src) {
    std::vector<T> dst;
    for (T item : src.as<JsonArrayConst>()) dst.push_back(item);
    return dst;
  }

  static bool checkJson(JsonVariantConst src) {
    JsonArrayConst array = src;
    bool result = array;
    for (JsonVariantConst item : array) result &= item.is<T>();
    return result;
  }
};
}  // namespace ARDUINOJSON_NAMESPACE

#endif  // SENSESP_SRC_SENSESP_TYPES_JSON_H_
