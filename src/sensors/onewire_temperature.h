#ifndef _onewire_H_
#define _onewire_H_

#include <set>

#include <DallasTemperature.h>
#include <OneWire.h>

#include "sensor.h"

typedef std::array<uint8_t, 8> OWDevAddr;

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

class OneWireTemperature : public NumericSensor {
 public:
  OneWireTemperature(DallasTemperatureSensors* dts, uint read_delay = 1000,
                     String config_path="");
  void enable() override final;
  virtual JsonObject& get_configuration(JsonBuffer& buf) override final;
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
