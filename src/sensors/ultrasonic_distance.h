#ifndef _ultrasonic_distance_H_
#define _ultrasonic_distance_H_
#include "sensor.h"

/**
 * @brief Reads a DYP-A02YYWM-V1.0 (and possibly other types) ultrasonic
 * sensor and outputs the number of microseconds that it takes the sound
 * wave to bounce off an object and return to the sensor.
 * 
 * Output must be sent to a Linear Transform to convert to a distance.
 * 
 * @see https://github.com/SignalK/SensESP/tree/master/examples/ultrasonic_level_sensor
 * 
 * @param trigger_pin The pin that is set to HIGH/LOW to start/stop the measurement.
 * 
 * @param input_pin The pin that reads the measurement
 * 
 * @param read_delay The time between reads of the sensor, in ms. Defaults to 1000.
 * 
 * @param config_path The path to configure read_delay in the Config UI.
 **/
class UltrasonicDistance : public NumericSensor {
 public:
  UltrasonicDistance(int8_t trigger_pin, int8_t input_pin,
                     uint read_delay = 1000, String config_path = "");
  void enable() override final;

 private:
  int8_t trigger_pin;
  int8_t input_pin;
  uint read_delay;
  virtual void get_configuration(JsonObject& doc) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;
};

// FIXME: Uncomment the following once the PIO Xtensa toolchain is updated
// [[deprecated("Use UltrasonicDistance instead.")]]
typedef UltrasonicDistance UltrasonicSens;

#endif
