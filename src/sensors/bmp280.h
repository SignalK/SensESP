#ifndef _bmp280_H_
#define _bmp280_H_

#include <Adafruit_BMP280.h>
#include <Wire.h>

#include "sensor.h"

// The BMP280 classes are based on the ADAfruit_BMP280 library.

/** 
 * @brief BMP280 represents an ADAfruit or compatible BMP280 temperature & pressure
 * sensor. 
 * 
 * The constructor creates a pointer to the instance, and starts up the
 * sensor. The pointer is passed to BMP280Value, which retrieves the specified
 * value. If you want to change any of the values with the
 * Adafruit_BMP280::setSampling() method, it's public, so you can call that
 * after you instantiate the BMP280 and before you start using it, with:
 * yourInstanceVariable->bmp280->setSampling(); See the Adafruit library for
 * details.
 * https://github.com/adafruit/Adafruit_BMP280_Library/blob/master/Adafruit_BMP280.h
 * 
 * @param addr The memory address of the sensor. Defaults to 0x77, but other
 * addresses are available for some BME280 chips. See the datasheet.
 * 
 * @param sensor A pointer to the actual BME280 sensor, returned by a constructor to
 * an Adafruit_BMP280 object. By default, SensESP uses the default Adafruit_BMP280
 * constructor - the one with no parameters. If your ESP doesn't use standard I2C pins
 * for SDA and SCL, you'll need to do something like this in main.cpp:
 *   Wire.begin(SDA_PIN, SCL_PIN, FREQUENCY); // ESP32 uses FREQUENCY, ESP8266 does not
 *   auto* bmp280 = new BMP280(0x76, "/some_config/path", new Adafruit_BMP280(&Wire));
*/
class BMP280 : public Sensor {
 public:
  BMP280(uint8_t addr = 0x77, Adafruit_BMP280* sensor = NULL);
  Adafruit_BMP280* adafruit_bmp280_;
};


/**
 * @brief BMP280Value reads and outputs the specified value of a BMP280 sensor.
 * 
 * @param bmp280 A pointer to an instance of a BMP280.
 * 
 * @param val_type The type of value you're reading: temperature or pressure.
 * 
 * @param read_delay How often to read the sensor - in ms.
 * 
 * @param config_path Path in the Config UI to configure read_delay
 */
class BMP280Value : public NumericSensor {
 public:
  
  enum BMP280ValType { temperature, pressure };
  BMP280Value(BMP280* bmp280, BMP280ValType val_type, uint read_delay = 500,
              String config_path = "");
  void enable() override final;
  BMP280* bmp280_;

 private:
  BMP280ValType val_type_;
  uint read_delay_;
  virtual void get_configuration(JsonObject& doc) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;
};

// FIXME: Uncomment the following once the PIO Xtensa toolchain is updated
// [[deprecated("Use BMP280Value instead.")]]
typedef BMP280Value BMP280value;

#endif
