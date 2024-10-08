#ifndef SENSESP_TRANSFORMS_INTEGRATOR_H_
#define SENSESP_TRANSFORMS_INTEGRATOR_H_

#include "sensesp/ui/config_item.h"
#include "transform.h"

namespace sensesp {

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
    this->load();
    this->emit(value);
  }

  virtual void set(const C& input) override final {
    value += input * k;
    this->emit(value);
  }

  void reset() { value = 0; }

  virtual bool to_json(JsonObject& doc) override final {
    doc["k"] = k;
    return true;
  }
  virtual bool from_json(const JsonObject& config) override final {
    if (!config["k"].is<P>()) {
      return false;
    }
    k = config["k"];
    return true;
  }

 private:
  P k;
  P value = 0;
};

template <typename T>
const String ConfigSchema(const Integrator<T, T>& obj) {
  return R"({"type":"object","properties":{"k":{"title":"Multiplier","type":"number"}}})";
}

typedef Integrator<float, float> FloatIntegrator;
typedef Integrator<int, int> Accumulator;

}  // namespace sensesp
#endif
