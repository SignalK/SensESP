#ifndef _integrator_H_
#define _integrator_H_

#include "transform.h"

namespace sensesp {

static const char INTEGRATOR_SCHEMA[] PROGMEM = R"({
    "type": "object",
    "properties": {
        "k": { "title": "Multiplier", "type": "number" }
    }
  })";

/**
 * @brief Integrator integrates (accumulates) the incoming values.
 *
 * The integrator output value is the sum of the all previous values plus
 * the latest value, multiplied by the coefficient k.
 *
 * @tparam C Consumer (incoming) data type
 * @tparam P Producer (output) data type
 */
template <class C, class P>
class Integrator : public Transform<C, P> {
 public:
  /**
   * @brief Construct a new Integrator T object
   *
   * @param k Multiplier coefficient
   * @param value Initial value of the accumulator
   * @param config_path Configuration path
   */
  Integrator(P k = 1, P value = 0, String config_path = "")
      : Transform<C, P>(config_path), k{k}, value{value} {
    this->load_configuration();
    this->emit(value);
  }

  virtual void set(C input) override final {
    value += input * k;
    this->emit(value);
  }

  void reset() { value = 0; }

  virtual void get_configuration(JsonObject& doc) override final {
    doc["k"] = k;
  }
  virtual bool set_configuration(const JsonObject& config) override final {
    String expected[] = {"k"};
    for (auto str : expected) {
      if (!config.containsKey(str)) {
        return false;
      }
    }
    k = config["k"];
    return true;
  }
  virtual String get_config_schema() override {
    return FPSTR(INTEGRATOR_SCHEMA);
  }

 private:
  P k;
  P value = 0;
};

typedef Integrator<float, float> FloatIntegrator;
typedef Integrator<int, int> Accumulator;

}  // namespace sensesp
#endif
