#ifndef _sht31_H_
#define _sht31_H_

#include <Adafruit_SHT31.h>
#include <Wire.h>

#include "sensor.h"

/* The SHT31 classes are based on the ADAfruit_SHT31 library. */

/** 
 * @brief SHT31 represents an ADAfruit (or compatible) SHT31 temperature & humidity
 * sensor. 
 * 
 * The constructor creates a pointer to the sensor, and starts up the
 * sensor. The pointer is passed to SHT31value, which retrieves the specified
 * value.
 * 
 * @param addr The memory address of the sensor. Defaults to 0x44, but other
 * addresses are available for some SHT31 chips. See the datasheet.
**/

class SHT31 {
 public:
  SHT31(uint8_t addr = 0x44);
  Adafruit_SHT31* adafruit_sht31_;
};


/** 
 * @brief SHT31Value reads and outputs the specified value of an SHT31 sensor
 * 
 * @param sht31 A pointer to an instance of a SHT31.
 * 
 * @param val_type The type of value you're reading: temperature or humidity.
 * 
 * @param read_delay How often to read the sensor - in ms.
 * 
 * @param config_path Path in the Config UI to configure read_delay
 **/
class SHT31Value : public NumericSensor {
 public:
  enum SHT31ValType { temperature, humidity };
  SHT31Value(SHT31* sht31, SHT31ValType val_type, uint read_delay = 500,
             String config_path = "");
  void enable() override final;
  SHT31* sht31_;

 private:
  SHT31ValType val_type_;
  uint read_delay_;
  virtual void get_configuration(JsonObject& doc) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;
};

// FIXME: Uncomment the following once the PIO Xtensa toolchain is updated
// [[deprecated("Use SHT31Value instead.")]]
typedef SHT31Value SHT31value;

#endif
