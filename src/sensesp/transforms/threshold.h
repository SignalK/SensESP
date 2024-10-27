#include "transform.h"
#ifndef _threshold_h
#define _threshold_h

namespace sensesp {

/**
 * @brief A Transform base class that translates the value of type C into
 * boolean. Base class for classes FloatThreshold and IntThreshold.
 *
 *   @param min_value Minimum value of input for output to be the value of
 * in_range.
 *
 *   @param max_value Maximum value of input for output to be the value of
 * in_range.
 *
 *   @param in_range Output value if input value is in range.
 *
 *   @param out_range Output value if input value is out of the range.
 */
template <typename C>
class ThresholdTransform : public Transform<C, bool> {
 public:

  ThresholdTransform(C min_value, C max_value, bool in_range,
                     String config_path = "")
      : Transform<C, bool>(config_path),
        min_value_{min_value},
        max_value_{max_value},
        in_range_{in_range} {
    this->load();
  };

  virtual void set(const C& new_value) override {
    if (new_value >= min_value_ && new_value <= max_value_) {
      this->output_ = in_range_;
    } else {
      this->output_ = !in_range_;
    }

    this->notify();
  }

  bool from_json(const JsonObject& root) override {
    String expected[] = {"min", "max", "in_range", "out_range"};
    for (auto str : expected) {

      if (!root[str].is<JsonVariant>()) {
        return false;
      }
    }
    min_value_ = root["min"];
    max_value_ = root["max"];
    in_range_ = root["in_range"];
    return true;
  }

  bool to_json(JsonObject& root) override {
    root["min"] = min_value_;
    root["max"] = max_value_;
    root["in_range"] = in_range_;
    return true;
  }

 protected:
  C min_value_;
  C max_value_;
  bool in_range_;
};

const String ConfigSchema(const ThresholdTransform<float>& obj);
const String ConfigSchema(const ThresholdTransform<int>& obj);

typedef ThresholdTransform<float> FloatThreshold;
typedef ThresholdTransform<int> IntThreshold;

}  // namespace sensesp
#endif
