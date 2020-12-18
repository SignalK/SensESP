#ifndef _ina219_H_
#define _ina219_H_

#include <Adafruit_INA219.h>
#include <Wire.h>

#include "sensor.h"

/* The INA219 classes are based on the ADAfruit_INA219 library. */

/**
 * These are used to tell the constructor what to set for maximum voltage
 * and amperage. The default in the Adafruit constructor is 32V and 2A.
 **/

enum INA219CAL_t { cal32_2, cal32_1, cal16_400 };

/**
 * @brief  Represents an ADAfruit (or compatible) INA219 High Side DC Current
 * Sensor. 
 * 
 * The constructor creates a pointer to the sensor, and starts up the
 * sensor. The pointer is passed to INA219value, which retrieves the specified
 * value.
 * 
 * @param addr Memory address where the sensor is read. Defaults to 0x40. Some
 * sensors can use a different address - see the datasheet.
 * 
 * @param calibration_setting Determines the maximum voltage and amperage to be
 * read. Choices are cal32_2 (the default), cal32_1, and cal16_400. See the
 * Adafruit library for details. 
 * @see https://github.com/adafruit/Adafruit_INA219
 */
class INA219 : public Sensor {
 public:
  INA219(uint8_t addr = 0x40, INA219CAL_t calibration_setting = cal32_2);
  Adafruit_INA219* ada_ina219_;
};


/**
 * @brief INA219Value reads and outputs the specified value of an INA219 sensor.
 * 
 * @param ina219 A pointer to an instance of an INA219.
 * 
 * @param val_type The type of value you're reading:
 *      bus_voltage, shunt_voltage, current, power, or load_voltage.
 * 
 * @param read_delay How often to read the sensor, in ms. Default is 500.
 * 
 * @param config_path Path in the Config UI to configure read_delay
 */
class INA219Value : public NumericSensor {
 public:
  enum INA219ValType { bus_voltage, shunt_voltage, current, power, load_voltage };
  INA219Value(INA219* ina219, INA219ValType val_type, uint read_delay = 500,
              String config_path = "");
  void enable() override final;
  INA219* ina219;

 private:
  INA219ValType val_type;
  uint read_delay;
  virtual void get_configuration(JsonObject& doc) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;
};

// FIXME: Uncomment the following once the PIO Xtensa toolchain is updated
// [[deprecated("Use INA219Value instead.")]]
typedef INA219Value INA219value;

#endif
