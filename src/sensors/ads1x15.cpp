#include "ads1x15.h"

#include "sensesp.h"

template <class T_Ada_1x15>
ADS1x15<T_Ada_1x15>::ADS1x15(uint8_t addr, adsGain_t gain, String config_path)
    : Sensor(config_path), gain{gain} {
  ads = new T_Ada_1x15(addr);
  ads->begin();
  ads->setGain(gain);
}

// define all possible instances of an ADS1x15
template class ADS1x15<Adafruit_ADS1015>;
template class ADS1x15<Adafruit_ADS1115>;


template <class T_ads_1x15>
ADS1x15RawValue<T_ads_1x15>::ADS1x15RawValue(T_ads_1x15* ads1x15, uint8_t channel,
                                       uint read_delay, String config_path)
    : NumericSensor(config_path),
      ads1x15{ads1x15},
      channel{channel},
      read_delay{read_delay} {
  load_configuration();
}

template <class T_ads_1x15>
void ADS1x15RawValue<T_ads_1x15>::read_raw_value() {
  switch (channel) {
    case 0:
    case 1:
    case 2:
    case 3:
      raw_value = ads1x15->ads->readADC_SingleEnded(channel);
      break;
    case 10:
      raw_value = ads1x15->ads->readADC_Differential_0_1();
      break;
    case 23:
      raw_value = ads1x15->ads->readADC_Differential_2_3();
      break;
    default:
      debugE("FATAL: invalid channel - must be 0, 1, 2, 3, 10, or 23");
  }
}

template <class T_ads_1x15>
void ADS1x15RawValue<T_ads_1x15>::enable() {
  app.onRepeat(read_delay, [this]() {
    read_raw_value();
    this->emit(raw_value);
  });
}

template <class T_ads_1x15>
void ADS1x15RawValue<T_ads_1x15>::get_configuration(JsonObject& root) {
  root["read_delay"] = read_delay;
  root["value"] = output;
};

static const char SCHEMA[] PROGMEM = R"###({
    "type": "object",
    "properties": {
        "read_delay": { "title": "Read delay", "type": "number", "description": "The time, in milliseconds, between each read of the input" },
        "value": { "title": "Last value", "type" : "number", "readOnly": true }
    }
  })###";

template <class T_ads_1x15>
String ADS1x15RawValue<T_ads_1x15>::get_config_schema() {
  return FPSTR(SCHEMA);
}

template <class T_ads_1x15>
bool ADS1x15RawValue<T_ads_1x15>::set_configuration(const JsonObject& config) {
  String expected[] = {"read_delay"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  read_delay = config["read_delay"];
  return true;
}

// define all possible instances of an ADS1x15value
template class ADS1x15RawValue<ADS1015>;
template class ADS1x15RawValue<ADS1115>;



template <class T_ads_1x15, ADS1x15CHIP_t chip>
ADS1x15Voltage<T_ads_1x15, chip>::ADS1x15Voltage(T_ads_1x15* ads1x15, uint8_t channel,
                                           uint read_delay, String config_path)
    : ADS1x15RawValue<T_ads_1x15>(ads1x15, channel, read_delay, config_path), chip_type{chip} {
  ADS1x15RawValue<T_ads_1x15>::load_configuration();
}


template <class T_ads_1x15, ADS1x15CHIP_t chip>
void ADS1x15Voltage<T_ads_1x15, chip>::calculate_voltage(int input) {
  if (chip_type == ADS1015chip) {
    switch (ADS1x15RawValue<T_ads_1x15>::ads1x15->ads->gain) {
      case GAIN_TWOTHIRDS:
        calculated_voltage = input * 0.003;
        break;
      case GAIN_ONE:
        calculated_voltage = input * 0.002;
        break;
      case GAIN_TWO:
        calculated_voltage = input * 0.001;
        break;
      case GAIN_FOUR:
        calculated_voltage = input * 0.0005;
        break;
      case GAIN_EIGHT:
        calculated_voltage = input * 0.00025;
        break;
      case GAIN_SIXTEEN:
        calculated_voltage = input * 0.000125;
        break;
      default:
        debugE("FATAL: invalid GAIN parameter.");
    }
  } else if (chip_type == ADS1115chip) {
    switch (ADS1x15RawValue<T_ads_1x15>::ads1x15->ads->gain) {
      case GAIN_TWOTHIRDS:
        calculated_voltage = input * 0.0001875;
        break;
      case GAIN_ONE:
        calculated_voltage = input * 0.000125;
        break;
      case GAIN_TWO:
        calculated_voltage = input * 0.0000625;
        break;
      case GAIN_FOUR:
        calculated_voltage = input * 0.00003125;
        break;
      case GAIN_EIGHT:
        calculated_voltage = input * 0.000015625;
        break;
      case GAIN_SIXTEEN:
        calculated_voltage = input * 0.0000078125;
        break;
      default:
        debugE("FATAL: invalid GAIN parameter.");
    }
  } else {
    debugE("FATAL: chip parameter must be ADS1015chip or ADS1115chip.");
  }
}


template <class T_ads_1x15, ADS1x15CHIP_t chip>
void ADS1x15Voltage<T_ads_1x15, chip>::enable() {
  app.onRepeat(ADS1x15RawValue<T_ads_1x15>::read_delay, [this]() {
    ADS1x15RawValue<T_ads_1x15>::read_raw_value();
    calculate_voltage(ADS1x15RawValue<T_ads_1x15>::raw_value);
    this->emit(calculated_voltage);
  });
}

