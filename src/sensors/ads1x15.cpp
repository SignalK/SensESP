#include "ads1x15.h"

#include "sensesp.h"

template <class T_Ada_1x15>
ADS1x15<T_Ada_1x15>::ADS1x15(
    uint8_t addr, adsGain_t gain, String config_path)
    : Sensor(config_path) {
  className = "ADS1x15";
  ads = new T_Ada_1x15(addr);
  ads->begin();
  ads->setGain(gain);
  }

// define all possible instances of an ADS1x15
template class ADS1x15<Adafruit_ADS1015>;
template class ADS1x15<Adafruit_ADS1115>;


// ADS1015 is used to read a value from an ADS1015 ADC.
ADS1015channel::ADS1015channel(ADS1015* ads1015, uint8_t channel, uint read_delay, String config_path) :
                   NumericSensor(config_path), ads1015{ads1015}, channel{channel}, read_delay{read_delay} {
      className = "ADS1015channel";
}

void ADS1015channel::enable() {
  app.onRepeat(read_delay, [this](){ 
      switch (channel) { 
        case 0:
        case 1:
        case 2:
        case 3: output = ads1015->ads->readADC_SingleEnded(channel); 
                break; 
        case 10: output = ads1015->ads->readADC_Differential_0_1(); 
                 break; 
        case 23: output = ads1015->ads->readADC_Differential_2_3(); 
                break; 
        default: debugE("FATAL: invalid channel - must be 0, 1, 2, 3, 10, or 23");  
      }
      notify();
 });
}

JsonObject& ADS1015channel::get_configuration(JsonBuffer& buf) {
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


  String ADS1015channel::get_config_schema() {
  return FPSTR(SCHEMA);
}

bool ADS1015channel::set_configuration(const JsonObject& config) {
  String expected[] = {"read_delay"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  read_delay = config["read_delay"];
  return true;
}

// ADS1115 is used to read a value from an ADS1115 ADC.
ADS1115channel::ADS1115channel(ADS1115* ads1115, uint8_t channel, uint read_delay, String config_path) :
                   NumericSensor(config_path), ads1115{ads1115}, channel{channel}, read_delay{read_delay} {
      className = "ADS1115channel";
}

void ADS1115channel::enable() {
  app.onRepeat(read_delay, [this](){ 
      switch (channel) { 
        case 0:
        case 1:
        case 2:
        case 3: output = ads1115->ads->readADC_SingleEnded(channel); 
                break; 
        case 10: output = ads1115->ads->readADC_Differential_0_1(); 
                 break; 
        case 23: output = ads1115->ads->readADC_Differential_2_3(); 
                break; 
        default: debugE("FATAL: invalid channel - must be 0, 1, 2, 3, 10, or 23");  
      }
      notify();
 });
}

JsonObject& ADS1115channel::get_configuration(JsonBuffer& buf) {
  JsonObject& root = buf.createObject();
  root["read_delay"] = read_delay;
  root["value"] = output;
  return root;
  };

  static const char SCHEMA2[] PROGMEM = R"###({
    "type": "object",
    "properties": {
        "read_delay": { "title": "Read delay", "type": "number", "description": "The time, in milliseconds, between each read of the input" },
        "value": { "title": "Last value", "type" : "number", "readOnly": true }
    }
  })###";


  String ADS1115channel::get_config_schema() {
  return FPSTR(SCHEMA2);
}

bool ADS1115channel::set_configuration(const JsonObject& config) {
  String expected[] = {"read_delay"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  read_delay = config["read_delay"];
  return true;
}