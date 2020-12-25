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
 * @param drdy_pin The data-ready pin. Not used, but still provided for backward-
 * compatibility.
 * @param tc_type Type of thermocouple that's attached to the MAX31856. For the
 * available types, see 
 * https://github.com/adafruit/Adafruit_MAX31856/blob/master/Adafruit_MAX31856.h
 * @param read_delay How often to read the sensor, in ms.
 * @param config_path The path to the sensor in the Config UI.
 **/
class MAX31856Thermocouple : public NumericSensor {
 public:
  MAX31856Thermocouple(int8_t cs_pin, int8_t mosi_pin, int8_t miso_pin,
                       int8_t clk_pin, int8_t drdy_pin,
                       max31856_thermocoupletype_t tc_type, uint read_delay,
                       String config_path = "");
  
/**
 * @param max31856 A pointer to an Adafruit_MAX31856, which you must create in
 * main.cpp before you call this constructor. After you create it, you must call
 * begin(), and also set the thermocouple type, before calling this constructor:
   
   auto* max = new Adafruit_MAX31856( <<whatever parameters you want / need>> );
   max->begin();
   max->setThermocoupleType( <<whatever type you're using>> );
   // now you can call this constructor
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
  virtual void get_configuration(JsonObject& doc) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;
};

#endif
