#ifndef _ultrasonic_input_H_
#define _ultrasonic_input_H_
#include "sensor.h"

class UltrasonicSens : public Sensor {
  public:
    UltrasonicSens(int8_t trigger_pin, int8_t input_pin, String config_path = "");
};

class UltrasonicSensValue : public NumericSensor {
  public:
    UltrasonicSensValue(UltrasonicSens* pUltrasonicSens, uint read_delay = 500, String config_path = "");
    void enable() override final;
    UltrasonicSens* pUltrasonicSens;

 private:

  uint read_delay;
  virtual JsonObject& get_configuration(JsonBuffer& buf) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;
//  void update();
};

#endif
