#ifndef _onewire_H_
#define _onewire_H_

#include <set>

#include <DallasTemperature.h>
#include <OneWire.h>

#include "sensor.h"

typedef std::array<uint8_t, 8> OWDevAddr;

/**
 * @brief Finds all 1-Wire temperature sensors that are connected
 * to the ESP and makes the address(es) and temperature data available
 * to the OneWireTemperature class.
 * 
 * This class has no pre-defined limit on the number of 1-Wire sensors
 * it can work with. From a practical standpoint, it should handle all
 * the sensors you're likely to connect to a single ESP.
 * 
 * @param pin The GPIO pin to which you have the data wire of the
 * 1-Wire sensor(s) connected.
 * 
 * @param config_path Currently not used for this class, don't provide
 * it - it defaults to a blank String.
 **/ 
class DallasTemperatureSensors : public Sensor {
 public:
  DallasTemperatureSensors(int pin, String config_path="");
  void enable() override final {}
  bool register_address(const OWDevAddr& addr);
  bool get_next_address(OWDevAddr* addr);
  DallasTemperature* sensors;
 private:
  OneWire* onewire;
  uint8_t next_sensor = 0;
  std::set<OWDevAddr> known_addresses;
  std::set<OWDevAddr> registered_addresses;
};

/**
 * @brief Used to read the temperature from a single 1-Wire temperature
 * sensor. If you have X sensors connected to your ESP, you need to create
 * X instances of this class in main.cpp.
 * 
 * All instances of this class have
 * a pointer to the same instance of DallasTemperatureSensors.
 * 
 * Temperature is read in Celsius, then converted to Kelvin before sending
 * to Signal K.
 * 
 * It will use the next available sensor address from all those found by the
 * DallasTemperatureSensors class. Once a sensor is "registered" by this class,
 * it will keep reading that sensor (by its unique hardware address) unless you
 * change the address in the Config UI.
 * 
 * @see https://github.com/SignalK/SensESP/tree/master/examples/thermocouple_temperature_sensor
 * 
 * @param dts Pointer to an instance of a DallasTemperatureSensors class.
 * 
 * @param read_delay How often to read the temperature. It takes up to 750 ms for the data
 * to be read by the chip, so this parameter should not be less than 750. You should
 * probably make it 1000 or more, to be safe.
 * 
 * @param config_path The path to configure the sensor address in the Config UI.
 **/ 
class OneWireTemperature : public NumericSensor {
 public:
  OneWireTemperature(DallasTemperatureSensors* dts, uint read_delay = 1000,
                     String config_path="");
  void enable() override final;
  virtual void get_configuration(JsonObject& doc) override final;
  virtual bool set_configuration(const JsonObject& config) override final;
  virtual String get_config_schema() override;

 private:
  OneWire* onewire;
  DallasTemperatureSensors* dts;
  uint read_delay;
  bool found = true;
  OWDevAddr address = {};
  void update();
  void read_value();
};

#endif
