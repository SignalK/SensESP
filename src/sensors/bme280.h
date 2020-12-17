#ifndef _bme280_H_
#define _bme280_H_

#include <Adafruit_BME280.h>
#include <Wire.h>

#include "sensor.h"

// The BME280 classes are based on the ADAfruit_BME280 library.

/**
 * @brief Represents an ADAfruit (or compatible) BME280 temperature / pressure /
 * humidity sensor.
 * 
 * The constructor creates a pointer to the sensor and
 * starts up the sensor. The pointer is passed to BME280value, which retrieves
 * the specified value. If you want to change any of the values with the
 * Adafruit_BME280::setSampling() method, it's public, so you can call that
 * after you instantiate the BME280 and before you start using it, with:
 * sensor_object->adafruit_bme280->setSampling(); See the Adafruit
 * library for details.
 * @see https://github.com/adafruit/Adafruit_BME280_Library/blob/master/Adafruit_BME280.h
 * 
 * @param addr The memory address where the sensor can be read. Default is 0x77. Some
 * sensors use, or can use, different addresses - check your datasheet.
 **/
class BME280 : public Sensor {
 public:
  BME280(uint8_t addr = 0x77, String config_path = "");
  Adafruit_BME280* adafruit_bme280_;

 private:
  uint8_t addr_;
  void check_status();
};


/** 
 * @brief BME280Value reads and outputs the specified value of a BME280 sensor
 * 
 * @param bme280 A pointer to an instance of a BME280.
 * 
 * @param val_type The type of value you're reading: temperature, pressure, or
 * humidity.
 * 
 * @param read_delay How often to read the sensor - in ms.
 * 
 * @param config_path Path in the Config UI to configure read_delay
 **/ 
class BME280Value : public NumericSensor {
 public:
  enum BME280ValType { temperature, pressure, humidity };
  BME280Value(BME280* bme280, BME280ValType val_type, uint read_delay = 500,
              String config_path = "");
  void enable() override final;
  BME280* bme280_;

 private:
  BME280ValType val_type_;
  uint read_delay_;
  virtual void get_configuration(JsonObject& doc) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;
};

// FIXME: Uncomment the following once the PIO Xtensa toolchain is updated
// [[deprecated("Use BME280Value instead.")]]
typedef BME280Value BME280value;

#endif
