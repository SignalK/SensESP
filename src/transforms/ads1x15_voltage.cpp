#include "ads1x15_voltage.h"

ADS1x15Voltage::ADS1x15Voltage(uint8_t chip, adsGain_t gain, String config_path) :
       NumericTransform(config_path), chip{chip}, gain{gain} {
     className = "ADS1x15Voltage";
}

void ADS1x15Voltage::set_input(float input, uint8_t inputChannel) {
    if (chip == 0) { // ADS1015
      switch (gain) { 
        case GAIN_TWOTHIRDS: output = input * 0.003;
                break;
        case GAIN_ONE: output = input * 0.002;
                break;
        case GAIN_TWO: output = input * 0.001;
                break;
        case GAIN_FOUR: output = input * 0.0005; 
                break; 
        case GAIN_EIGHT: output = input * 0.00025; 
                 break; 
        case GAIN_SIXTEEN: output = input * 0.000125; 
                break; 
        default: debugE("FATAL: invalid GAIN parameter.");  
      }
    }
    else if (chip == 1) { // ADS1115
       switch (gain) { 
        case GAIN_TWOTHIRDS: output = input * 0.0001875;
                break;
        case GAIN_ONE: output = input * 0.000125;
                break;
        case GAIN_TWO: output = input * 0.0000625;
                break;
        case GAIN_FOUR: output = input * 0.00003125; 
                break; 
        case GAIN_EIGHT: output = input * 0.000015625; 
                 break; 
        case GAIN_SIXTEEN: output = input * 0.0000078125; 
                break; 
        default: debugE("FATAL: invalid GAIN parameter.");  
      } 
    }
    else {
      debugE("FATAL: chip parameter must be 0 (ADS1015) or 1 (ADS1115");
    }
    notify();
}

JsonObject& ADS1x15Voltage::get_configuration(JsonBuffer& buf) {
  JsonObject& root = buf.createObject();
  root["chip"] = chip;
  root["value"] = output;
  return root;
  };

  static const char SCHEMA[] PROGMEM = R"###({
    "type": "object",
    "properties": {
        "chip": { "title": "chip", "type": "number", "description": "Chip: 0 = ADS1015, 1 = ADS1115" },
        "value": { "title": "Last value", "type" : "number", "readOnly": true }
    }
  })###";

  String ADS1x15Voltage::get_config_schema() {
  return FPSTR(SCHEMA);
}

bool ADS1x15Voltage::set_configuration(const JsonObject& config) {
  String expected[] = {"chip"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  chip = config["chip"];
  return true;
}
