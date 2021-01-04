#ifndef _MAX31856_thermocouple_H_
#define _MAX31856_thermocouple_H_
#include "Adafruit_MAX31856.h"
#include "sensor.h"


/**
 * @brief Reads the temperature from an Adafruit MAX31856 (or compatible) thermocouple
 * breakout board. Communicates with the ESP via SPI.
 *  
 * @param cs_pin
 * @param mosi_pin
 * @param miso_pin
 * @param clk_pin
 * @param drdy_pin The data-ready pin. No longer used, but still provided for backward-
 * compatibility.
 * 
 * @param tc_type Type of thermocouple that's attached to the MAX31856. For the
 * available types, see 
 * https://github.com/adafruit/Adafruit_MAX31856/blob/master/Adafruit_MAX31856.h
 * 
 * @param read_delay How often to read the sensor, in ms.
 * @param config_path The path to the sensor in the Config UI.
 **/
class MAX31856Thermocouple : public NumericSensor {
 public:
  /* DEPRECATED: drdy_pin no longer used */
  MAX31856Thermocouple(int8_t cs_pin, int8_t mosi_pin, int8_t miso_pin,
                       int8_t clk_pin, int8_t drdy_pin,
                       max31856_thermocoupletype_t tc_type, uint read_delay,
                       String config_path = "");

  /**
   * A version of the constructor that doesn't require the now-obsolete drdy_pin.
  **/ 
  MAX31856Thermocouple(int8_t cs_pin, int8_t mosi_pin, int8_t miso_pin,
                       int8_t clk_pin, max31856_thermocoupletype_t tc_type,
                       uint read_delay, String config_path = "") :
                       MAX31856Thermocouple(cs_pin, mosi_pin, miso_pin,
                       clk_pin, 0, tc_type, read_delay, config_path) {}

  /**
   * A version of the constructor that allows you to create the Adafruit_MAX31856
   * in main.cpp, with whatever paramaters you need, and then point to it.
   * 
   * @param max31856 A pointer to an Adafruit_MAX31856, which you must create in
   * main.cpp before you call this constructor. After you create it, you must call
   * begin(), and set the thermocouple type, before calling this constructor,
   * like this:

     auto* max = new Adafruit_MAX31856( <<whatever parameters you want / need>> );
     max->begin();
     max->setThermocoupleType( <<whatever type you're using>> );
     // now you can call this constructor - 1000 ms read_delay shown here
     MAX31856Thermocouple(max, 1000, "/config/path");

   * @param read_delay How often to read the sensor, in ms.
   * @param config_path The path to configure read_delay in the Config UI.
  **/
  MAX31856Thermocouple(Adafruit_MAX31856* max31856, uint read_delay = 500,
                       String config_path = "");                     
  Adafruit_MAX31856* max31856_;
  void enable() override final;

 private:
  int8_t data_ready_pin_;
  uint read_delay_;
  bool sensor_detected_ = true;
  virtual void get_configuration(JsonObject& doc) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;
};

#endif
