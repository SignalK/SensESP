#include "ina219.h"

#include "sensesp.h"
//#include "i2c_tools.h"
#include <RemoteDebug.h>


// INA219 represents an ADAfruit (or compatible) INA219 High Side DC Current Sensor.
INA219::INA219(uint8_t addr, INA219CAL_t calibration_setting, String config_path) :
       Sensor(config_path) {
    className = "INA219";
    load_configuration();
    pAdafruitINA219 = new Adafruit_INA219(addr);
    pAdafruitINA219->begin();
    // Default calibration in the Adafruit_INA219 constructor is 32V and 2A, so that's what it will be unless
    // it's set to something different in the call to this constructor:
    if (calibration_setting == cal32_1) {
         pAdafruitINA219->setCalibration_32V_1A();
    }     
    else if (calibration_setting == cal16_400) {
         pAdafruitINA219->setCalibration_16V_400mA();
    }     
}



// INA219value reads and outputs the specified type of value of a INA219 sensor
INA219value::INA219value(INA219* pINA219, INA219ValType val_type, uint read_delay, String config_path) :
                   NumericSensor(config_path), pINA219{pINA219}, val_type{val_type}, read_delay{read_delay} {
      className = "INA219value";
      load_configuration();
}


void INA219value::enable() {
  app.onRepeat(read_delay, [this](){
      switch (val_type) { 
        case bus_voltage: output = pINA219->pAdafruitINA219->getBusVoltage_V();
                break;
        case shunt_voltage: output = (pINA219->pAdafruitINA219->getShuntVoltage_mV() / 1000); // SK wants volts, not mV
                break;
        case current: output = (pINA219->pAdafruitINA219->getCurrent_mA() / 1000); // SK wants amps, not mA
                break;
        case power: output = (pINA219->pAdafruitINA219->getPower_mW() / 1000); // SK want watts, not mW
                break; 
        case load_voltage: output = (pINA219->pAdafruitINA219->getBusVoltage_V() + (pINA219->pAdafruitINA219->getShuntVoltage_mV() / 1000));
                 break; 
        default: debugE("FATAL: invalid val_type parameter.");  
      }
      
      notify();
 });
}

JsonObject& INA219value::get_configuration(JsonBuffer& buf) {
  JsonObject& root = buf.createObject();
  root["read_delay"] = read_delay;
  root["value"] = output;
  return root;
  };

  static const char SCHEMA[] PROGMEM = R"###({
    "type": "object",
    "properties": {
        "read_delay": { "title": "Read delay", "type": "number", "description": "The time, in milliseconds, between each read of the input" },
        "value": { "title": "Last value", "type" : "number", "readOnly": true }
    }
  })###";


  String INA219value::get_config_schema() {
  return FPSTR(SCHEMA);
}

bool INA219value::set_configuration(const JsonObject& config) {
  String expected[] = {"read_delay"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  read_delay = config["read_delay"];
  return true;
}
