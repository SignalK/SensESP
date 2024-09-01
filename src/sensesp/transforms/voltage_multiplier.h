#ifndef SENSESP_TRANSFORMS_VOLTAGE_MULTIPLIER_H_
#define SENSESP_TRANSFORMS_VOLTAGE_MULTIPLIER_H_

#include "transform.h"

namespace sensesp {

/**
 * @brief OBSOLETE: Since you can now apply an "output_scale" to the
 * AnalogInput sensor, you don't need this transform anymore. See the
 * description of "output_scale" in analog_input.h.
 *
 * A transform that does the opposite of what a physical voltage divider circuit
 * does: convert the voltage that comes out of a voltage divider back
 * into the original voltage that went into the voltage divider.
 *
 * Use case: measuring a ~12V DC source with an analog-to-digital converter
 * (ADC) that can take only 5V. You use a physical voltage divider circuit to
 * reduce from ~12V down to a range that's no higher than 5V. After you read the
 * output from the ADC and convert it into voltage, you have to reverse the
 * effect of the voltage divider circuit to know the original source voltage.
 *
 * @param R1 The value, in ohms, of R1 in your voltage divider circuit.
 *
 * @param R2 The value, in ohms, of R2 in your voltage divider circuit.
 *
 * For maximum accuracy, you should measure the actual ohms value of your
 * resistors, rather than just assuming they are as labeled.
 */
class VoltageMultiplier : public FloatTransform {
 public:
  VoltageMultiplier(uint16_t R1, uint16_t R2, const String& config_path = "");

  virtual void set(const float& input);

 private:
  uint16_t R1_{};
  uint16_t R2_{};
  virtual void get_configuration(JsonObject& root) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;
};

}  // namespace sensesp
#endif
