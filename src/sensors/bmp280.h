#ifndef _bmp280_H_
#define _bmp280_H_

#include <Adafruit_BMP280.h>
#include <Wire.h>

#include "sensor.h"

// The BMP280 classes are based on the ADAfruit_BMP280 library.

// BMP280 represents an ADAfruit (or compatible) BMP280 temperature & pressure
// sensor. The constructore creates a pointer to the instance, and starts up the
// sensor. The pointer is passed to BMP280value, which retrieves the specified
// value. If you want to change any of the values with the
// Adafruit_BMP280::setSampling() method, it's public, so you can call that
// after you instantiate the BMP280 and before you start using it, with:
// yourInstanceVariable->bmp280->setSampling(); See the Adafruit library for
// details.
// https://github.com/adafruit/Adafruit_BMP280_Library/blob/master/Adafruit_BMP280.h
class BMP280 : public Sensor {
 public:
  BMP280(uint8_t addr = 0x77, String config_path = "");
  Adafruit_BMP280* adafruit_bmp280;

 private:
  uint8_t addr;
};


// BMP280Value reads and outputs the specified value of a BMP280 sensor.
class BMP280Value : public NumericSensor {
 public:
  
  enum BMP280ValType { temperature, pressure };
  BMP280Value(BMP280* bmp280, BMP280ValType val_type, uint read_delay = 500,
              String config_path = "");
  void enable() override final;
  BMP280* bmp280;

 private:
  BMP280ValType val_type;
  uint read_delay;
  virtual void get_configuration(JsonObject& doc) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;
};

// FIXME: Uncomment the following once the PIO Xtensa toolchain is updated
// [[deprecated("Use BMP280Value instead.")]]
typedef BMP280Value BMP280value;

#endif
