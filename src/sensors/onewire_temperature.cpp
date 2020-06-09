#include "onewire_temperature.h"

#include <algorithm>

#include <DallasTemperature.h>
#include <OneWire.h>

#include "sensor.h"
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
    int pin, String config_path)
    : Sensor(config_path) {
  className = "DallasTemperatureSensor";
  onewire = new OneWire(pin);
  sensors = new DallasTemperature(onewire);
  sensors->begin();

  // DallasTemperature::getDeviceCount() doesn't work reliably with ESP32,
  // so sensors are found using getAddress()

  DeviceAddress addr;
  OWDevAddr owda;
  bool check_again = true;
  uint8_t sensor_index = 0;
  while (check_again) {
    if(sensors->getAddress(addr, sensor_index)) {
      std::copy(std::begin(addr), std::end(addr), std::begin(owda));
      known_addresses.insert(owda);
      #ifndef DEBUG_DISABLED
      char addrstr[24];
      owda_to_string(addrstr, owda);
      debugI("Found OneWire sensor %s", addrstr);
      #endif
      sensor_index++;
    }
    else check_again = false;
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
    DallasTemperatureSensors* dts, uint read_delay, String config_path)
    : NumericSensor(config_path), dts{dts}, read_delay{read_delay}{
  className = "OneWireTemperature";
  load_configuration();
  if (address==null_ow_addr) {
    // previously unconfigured sensor
    bool success = dts->get_next_address(&address);
    if (!success) {
      debugE("FATAL: Unable to allocate a OneWire sensor for %s. "
             "All sensors have already been configured. "
             "Check the physical wiring of your sensors.",
             config_path.c_str());
      found = false;
    } else {
      debugD("Registered a new OneWire sensor");
      dts->register_address(address);
    }
  } else {
    bool success = dts->register_address(address);
    if (!success) {
      char addrstr[24];
      owda_to_string(addrstr, address);
      debugE("FATAL: OneWire sensor %s at %s is missing. "
             "Check the physical wiring of your sensors.",
             config_path.c_str(), addrstr);
      found = false;
    }
  }
}

void OneWireTemperature::enable() {
  if (found) {
    app.onRepeat(read_delay, [this](){ this->update(); });
  }
}

void OneWireTemperature::update() {
  dts->sensors->requestTemperaturesByAddress(address.data());

  app.onDelay(750, [this](){ this->read_value(); });
}

void OneWireTemperature::read_value() {
  // getTempC returns degrees Celsius but SignalK expects Kelvins
  output = dts->sensors->getTempC(address.data()) + 273.15;
  this->notify();
}

JsonObject& OneWireTemperature::get_configuration(JsonBuffer& buf) {
  JsonObject& root = buf.createObject();
  root.set("value", output);
  char addr_str[24];
  owda_to_string(addr_str, address);
  root.set("address", addr_str);
  root.set("found", found);
  return root;
}

static const char SCHEMA[] PROGMEM = R"({
    "type": "object",
    "properties": {
        "address": { "title": "OneWire address", "type": "string" },
        "found": { "title": "Device found", "type": "boolean", "readOnly": true },
        "value": { "title": "Last value", "type" : "number", "readOnly": true }
    }
  })";

String OneWireTemperature::get_config_schema() {
  return FPSTR(SCHEMA);
}

bool OneWireTemperature::set_configuration(const JsonObject& config) {
  if (!config.containsKey("address")) {
    return false;
  }
  string_to_owda(&address, config["address"]);
  return true;
}
