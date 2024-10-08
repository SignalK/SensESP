#ifndef SENSESP_TRANSFORMS_VOLTAGE_DIVIDER_H_
#define SENSESP_TRANSFORMS_VOLTAGE_DIVIDER_H_

#include "transform.h"
#include "sensesp/ui/config_item.h"

namespace sensesp {

/**
 * @brief Uses the voltage divider formula to calculate (and output) the
 * resistance of R1 in the circuit.
 *
 * Vout = (Vin x R2) / (R1 + R2) is the voltage divider formula. We know:
 * - Vout - that's the input to this transform, probably coming from an
 * AnalogVoltage transform, or directly from an AnalogInput sensor.
 * - Vin - that's one of the paramaters to this transform. It's a fixed voltage
 * that you know from your physical voltage divider circuit.
 * - R2 - also a parameter to this transform, and also from your physical
 * voltage divider.
 *
 * Knowing Vin, Vout, and R2, we can calculate R1 (which is what this transform
 * does).
 *
 * The purpose of this transform is to help determine the resistance value of
 * a physical sensor of the "variable resistor" type, such as a temperature
 * sensor, or an oil pressure sensor. If we know the resistance of the sensor,
 * we can then determine the temperature (or pressure, etc.) that the sensor
 * is reading, by connecting this transform's output to an instance of the
 * CurveInterpolator transform.
 *
 * @see
 * https://github.com/SignalK/SensESP/blob/master/examples/temperature_sender.cpp
 * */
class VoltageDividerR1 : public SymmetricTransform<float> {
 public:
  VoltageDividerR1(float R2, float Vin = 3.3, const String& config_path = "");

  virtual void set(const float& Vout) override;

  virtual bool to_json(JsonObject& root) override;
  virtual bool from_json(const JsonObject& config) override;

 protected:
  float R2_;
  float Vin_;
};

const String ConfigSchema(const VoltageDividerR1& obj);

/**
 * @brief Uses the voltage divider formula to calculate (and output) the
 * resistance of R2 in the circuit.
 *
 * Vout = (Vin x R2) / (R1 + R2) is the voltage divider formula. We know:
 * - Vout - that's the input to this transform, probably coming from an
 * AnalogVoltage transform, or directly from an AnalogInput sensor.
 * - Vin - that's one of the paramaters to this transform. It's a fixed voltage
 * that you know from your physical voltage divider circuit.
 * - R1 - also a parameter to this transform, and also from your physical
 * voltage divider.
 *
 * Knowing Vin, Vout, and R1, we can calculate R2 (which is what this transform
 * does).
 *
 * The purpose of this transform is to help determine the resistance value of
 * a physical sensor of the "variable resistor" type, such as a temperature
 * sensor, or an oil pressure sensor. If we know the resistance of the sensor,
 * we can then determine the temperature (or pressure, etc.) that the sensor
 * is reading, by connecting this transform's output to an instance of the
 * CurveInterpolator transform.
 *
 * @see
 * https://github.com/SignalK/SensESP/blob/master/examples/temperature_sender.cpp
 */
class VoltageDividerR2 : public SymmetricTransform<float> {
 public:
  VoltageDividerR2(float R1, float Vin = 3.3, const String& config_path = "");

  virtual void set(const float& Vout) override;

  // For reading and writing the configuration of this transformation
  virtual bool to_json(JsonObject& root) override;
  virtual bool from_json(const JsonObject& config) override;

 protected:
  float R1_;
  float Vin_;
};

const String ConfigSchema(const VoltageDividerR2& obj);

}  // namespace sensesp
#endif
