#ifndef _integrator_H_
#define _integrator_H_

#include "transform.h"

static const char INTEGRATOR_SCHEMA[] PROGMEM = R"({
    "type": "object",
    "properties": {
        "k": { "title": "Multiplier", "type": "number" },
        "value": { "title": "Current value", "type" : "number", "readOnly": false }
    }
  })";

// y = k * sum(x_t)
template <class T>
class IntegratorT : public SymmetricTransform<T> {
 public:
  IntegratorT(T k = 1, T value = 0, String config_path = "")
      : SymmetricTransform<T>(config_path), k{k}, value{value} {
    this->load_configuration();
  }

  virtual void enable() override final {
    // save the integrator value every 10 s
    // NOTE: Disabled for now because interrupts start throwing
    // exceptions.
    // app.onRepeat(10000, [this](){ this->save_configuration(); });
  }

  virtual void set_input(T input, uint8_t inputChannel = 0) override final {
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
  virtual String get_config_schema() override { return FPSTR(INTEGRATOR_SCHEMA); }

 private:
  T value = 0;
  T k;
};

typedef IntegratorT<float> Integrator;
typedef IntegratorT<int> Accumulator;

#endif
