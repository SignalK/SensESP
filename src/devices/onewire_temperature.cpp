#include "onewire_temperature.h"

#include <algorithm>

#include <DallasTemperature.h>
#include <OneWire.h>

#include "device.h"
#include "sensesp.h"

const OWDevAddr null_ow_addr = {0, 0, 0, 0, 0, 0, 0, 0};

void owda_to_string(char* str, const OWDevAddr& addr) {
  // brute force it
  sprintf(str,
          "%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x",
          addr[0], addr[1], addr[2], addr[3],
          addr[4], addr[5], addr[6], addr[7]);
}

bool string_to_owda(OWDevAddr* addr, const char* str) {
  // brute force it
  uint vals[8];
  int num_items = sscanf(str,
                         "%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x",
                         &vals[0], &vals[1], &vals[2], &vals[3],
                         &vals[4], &vals[5], &vals[6], &vals[7]);
  for (int i=0; i<8; i++) {
    (*addr)[i] = vals[i];
  }
  return num_items==8;
}

DallasTemperatureSensors::DallasTemperatureSensors(
    int pin, String id, String schema)
    : Device{id, schema} {
  onewire = new OneWire(pin);
  sensors = new DallasTemperature(onewire);
  sensors->begin();

  int num_devices = sensors->getDeviceCount();
  debugI("OneWire devices found: %d", num_devices);

  DeviceAddress addr;
  OWDevAddr owda;
  for (int i=0; i<num_devices; i++) {
    sensors->getAddress(addr, i);
    std::copy(std::begin(addr), std::end(addr), std::begin(owda));
    known_addresses.insert(owda);
    #ifndef DEBUG_DISABLED
    char addrstr[24];
    owda_to_string(addrstr, owda);
    debugI("Found OneWire device %s", addrstr);
    #endif
  }

  // all conversions will by async
  sensors->setWaitForConversion(false);
  // always use maximum resolution
  sensors->setResolution(12);
}

bool DallasTemperatureSensors::register_address(const OWDevAddr& addr) {
  auto search_known = known_addresses.find(addr);
  if (search_known == known_addresses.end()) {
    // address is not known
    return false;
  }
  auto search_reg = registered_addresses.find(addr);
  if (search_reg != registered_addresses.end()) {
    // address is already registered
    return false;
  }

  registered_addresses.insert(addr);
  return true;
}

bool DallasTemperatureSensors::get_next_address(OWDevAddr* addr) {
  // find the next address from known_addresses which is
  // not present in registered_addresses
  for (auto known : known_addresses) {
    auto reg_it = registered_addresses.find(known);
    if (reg_it == registered_addresses.end()) {
      *addr = known;
      return true;
    }
  }
  return false;
}

OneWireTemperature::OneWireTemperature(
    DallasTemperatureSensors* dts, String id, String schema, uint8_t valueIndex)
    : NumericDevice{id, schema, valueIndex}, dts{dts} {
  load_configuration();
  if (address==null_ow_addr) {
    // previously unconfigured device
    bool success = dts->get_next_address(&address);
    if (!success) {
      debugE("FATAL: Could not find an available OneWire device");
      failed = true;
    } else {
      dts->register_address(address);
    }
  } else {
    bool success = dts->register_address(address);
    if (!success) {
      debugE("FATAL: Unable to register OneWire device");
      failed = true;
    }
  }
}

void OneWireTemperature::enable() {
  if (!failed) {
    app.onRepeat(1000, [this](){ this->update(); });
  }
}


void OneWireTemperature::update() {
  dts->sensors->requestTemperaturesByAddress(address.data());

  app.onDelay(750, [this](){ this->read_value(); });
}

void OneWireTemperature::read_value() {
  output = dts->sensors->getTempC(address.data());
  this->notify();
}

JsonObject& OneWireTemperature::get_configuration(JsonBuffer& buf) {
  JsonObject& root = buf.createObject();
  root.set("value", output);
  char addr_str[24];
  owda_to_string(addr_str, address);
  root.set("address", addr_str);
  root.set("failed", failed);
  return root;
}

bool OneWireTemperature::set_configuration(const JsonObject& config) {
  if (!config.containsKey("address")) {
    return false;
  }
  string_to_owda(&address, config["address"]);
  return true;
}
