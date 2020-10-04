#include "esp32_analog_input.h"

#include "Arduino.h"
#include "sensesp.h"
#include <RemoteDebug.h>

ESP32AnalogInput::ESP32AnalogInput(uint8_t pin, adc_bits_width_t adc_width, 
                                   adc_atten_t adc_attenuation,
                                   uint read_delay, String config_path)

    : NumericSensor(config_path), pin{pin}, read_delay{read_delay} {

  associatePin(pin, adc_width, adc_attenuation);
  className = "ESP32AnalogInput";
  load_configuration();
}

void ESP32AnalogInput::associatePin(int pin, adc_bits_width_t adc_width, 
                                    adc_atten_t adc_attenuation) {

  // Configure width across all channels on ADC1
  adc1_config_width(adc_width);
  
  // Set channel attenuation
  adc1_channel = (adc1_channel_t) digitalPinToAnalogChannel(pin);
  adc1_config_channel_atten(adc1_channel, adc_attenuation);

  // Characterize/calibrate the ADC
  cal_type = esp_adc_cal_characterize(ADC_UNIT_1, adc_attenuation, 
                                      adc_width, V_REF, &characteristics);
  
  if (cal_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
      cal_type_str = "eFuse Vref";
  } else if (cal_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
      cal_type_str = "Two Point";
  } else {
      cal_type_str = "Default";
  }

  // Setup the channel for calling the readVolts function
  adc_channel = (adc_channel_t) digitalPinToAnalogChannel(pin);
  
  configured = true;
}


float ESP32AnalogInput::readVolts() {
	uint32_t mV = 0;
	esp_adc_cal_get_voltage(adc_channel, &characteristics, &mV); // returns mV
	return  mV*0.001;
}

void ESP32AnalogInput::update() {
  //output = analogRead(pin);
  output = readVolts();
  this->notify();
}

void ESP32AnalogInput::enable() {
  app.onRepeat(read_delay, [this]() { this->update(); });
}

void ESP32AnalogInput::get_configuration(JsonObject& root) {
  root["read_delay"] = read_delay;
  root["value"] = output;
};

static const char SCHEMA[] PROGMEM = R"###({
    "type": "object",
    "properties": {
        "read_delay": { "title": "Read delay", "type": "number", "description": "Number of milliseconds between each analogRead(32)" },
        "value": { "title": "Last value", "type" : "number", "readOnly": true }
    }
  })###";

String ESP32AnalogInput::get_config_schema() { return FPSTR(SCHEMA); }

bool ESP32AnalogInput::set_configuration(const JsonObject& config) {
  String expected[] = {"read_delay"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  read_delay = config["read_delay"];
  return true;
}
