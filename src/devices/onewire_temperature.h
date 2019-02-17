#ifndef _onewire_H_
#define _onewire_H_

#include <set>

#include <DallasTemperature.h>
#include <OneWire.h>

#include "device.h"

typedef std::array<uint8_t, 8> OWDevAddr;

class DallasTemperatureSensors : public Device {
 public:
  DallasTemperatureSensors(int pin, String id="", String schema="");
  void enable() override final {}
  bool register_address(const OWDevAddr& addr);
  bool get_next_address(OWDevAddr* addr);
  DallasTemperature* sensors;
 private:
  OneWire* onewire;
  uint8_t next_device = 0;
  std::set<OWDevAddr> known_addresses;
  std::set<OWDevAddr> registered_addresses;
};

class OneWireTemperature : public Device {
 public:
  OneWireTemperature(DallasTemperatureSensors* dts,
                     String id="", String schema="");
  void enable() override final;
  float get();
  virtual JsonObject& get_configuration(JsonBuffer& buf) override final;
  virtual void set_configuration(const JsonObject& config) override final;
 private:
  OneWire* onewire;
  float value;
  DallasTemperatureSensors* dts;
  bool failed=false;
  OWDevAddr address = {};
  void update();
  void read_value();
};

#endif
