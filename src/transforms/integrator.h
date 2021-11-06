#ifndef _integrator_H_
#define _integrator_H_

#include "transform.h"

namespace sensesp {

static const char INTEGRATOR_SCHEMA[] PROGMEM = R"({
    "type": "object",
    "properties": {
        "k": { "title": "Multiplier", "type": "number" },
        "value": { "title": "Current value", "type" : "number", "readOnly": false }
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
class IntegratorT : public Transform<C, P> {
 public:
  /**
   * @brief Construct a new Integrator T object
   *
   * @param k Multiplier coefficient
   * @param value Initial value of the accumulator
   * @param config_path Configuration path
   */
  IntegratorT(P k = 1, P value = 0, String config_path = "")
      : Transform<C, P>(config_path), k{k}, value{value} {
    this->load_configuration();
  }

  virtual void start() override final {
    // save the integrator value every 10 s
    // NOTE: Disabled for now because interrupts start throwing
    // exceptions.
    // app.onRepeat(10000, [this](){ this->save_configuration(); });
  }

  virtual void set_input(C input, uint8_t inputChannel = 0) override final {
    value += input * k;
    this->emit(value);
  }

  void reset() { value = 0; }

  virtual void get_configuration(JsonObject& doc) override final {
    doc["k"] = k;
    doc["value"] = value;
  }
  virtual bool set_configuration(const JsonObject& config) override final {
    String expected[] = {"k"};
    for (auto str : expected) {
      if (!config.containsKey(str)) {
        return false;
      }
    }
    k = config["k"];
    value = config["value"];
    return true;
  }
  virtual String get_config_schema() override {
    return FPSTR(INTEGRATOR_SCHEMA);
  }

 private:
  P k;
  P value = 0;
};

typedef IntegratorT<float, float> Integrator;
typedef IntegratorT<int, int> Accumulator;

}  // namespace sensesp
#endif
