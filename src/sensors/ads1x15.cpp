#include "ads1x15.h"

#include "sensesp.h"

template <class T_Ada_1x15>
ADS1x15<T_Ada_1x15>::ADS1x15(uint8_t addr, adsGain_t gain, String config_path)
    : Sensor(config_path), gain_{gain} {
  ads_ = new T_Ada_1x15(addr);
  ads_->begin();
  ads_->setGain(gain_);
}

// define all possible instances of an ADS1x15
template class ADS1x15<Adafruit_ADS1015>;
template class ADS1x15<Adafruit_ADS1115>;


template <class T_ads_1x15>
ADS1x15RawValue<T_ads_1x15>::ADS1x15RawValue(T_ads_1x15* ads1x15, ADS1x15Channel_t channel,
                                       uint read_delay, String config_path)
    : NumericSensor(config_path),
      ads1x15_{ads1x15},
      channel_{channel},
      read_delay_{read_delay} {
  load_configuration();
}

template <class T_ads_1x15>
void ADS1x15RawValue<T_ads_1x15>::read_raw_value() {
  switch (channel_) {
    case channel_0:
    case channel_1:
    case channel_2:
    case channel_3:
      raw_value_ = ads1x15_->ads_->readADC_SingleEnded(channel_);
      break;
    case channels_0_1:
      raw_value_ = ads1x15_->ads_->readADC_Differential_0_1();
      break;
    case channels_2_3:
      raw_value_ = ads1x15_->ads_->readADC_Differential_2_3();
      break;
  }
}

template <class T_ads_1x15>
void ADS1x15RawValue<T_ads_1x15>::enable() {
  app.onRepeat(read_delay_, [this]() {
    read_raw_value();
    this->emit(raw_value_);
  });
}

template <class T_ads_1x15>
void ADS1x15RawValue<T_ads_1x15>::get_configuration(JsonObject& root) {
  root["read_delay"] = read_delay_;
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
  read_delay_ = config["read_delay"];
  return true;
}

// define all possible instances of an ADS1x15RawValue
template class ADS1x15RawValue<ADS1015>;
template class ADS1x15RawValue<ADS1115>;



template <class T_ads_1x15, ADS1x15CHIP_t chip>
ADS1x15Voltage<T_ads_1x15, chip>::ADS1x15Voltage(T_ads_1x15* ads1x15, ADS1x15Channel_t channel,
                                           uint read_delay, String config_path)
    : ADS1x15RawValue<T_ads_1x15>(ads1x15, channel, read_delay, config_path), chip_{chip} {
  ADS1x15RawValue<T_ads_1x15>::load_configuration();
}


template <class T_ads_1x15, ADS1x15CHIP_t chip>
void ADS1x15Voltage<T_ads_1x15, chip>::calculate_voltage(int input) {
  if (chip_ == ADS1015chip) {
    switch (ADS1x15RawValue<T_ads_1x15>::ads1x15_->gain_) {
      case GAIN_TWOTHIRDS:
        calculated_voltage_ = input * 0.003;
        break;
      case GAIN_ONE:
        calculated_voltage_ = input * 0.002;
        break;
      case GAIN_TWO:
        calculated_voltage_ = input * 0.001;
        break;
      case GAIN_FOUR:
        calculated_voltage_ = input * 0.0005;
        break;
      case GAIN_EIGHT:
        calculated_voltage_ = input * 0.00025;
        break;
      case GAIN_SIXTEEN:
        calculated_voltage_ = input * 0.000125;
        break;
      default:
        debugE("FATAL: invalid GAIN parameter.");
    }
  } else if (chip_ == ADS1115chip) {
    switch (ADS1x15RawValue<T_ads_1x15>::ads1x15_->gain_) {
      case GAIN_TWOTHIRDS:
        calculated_voltage_ = input * 0.0001875;
        break;
      case GAIN_ONE:
        calculated_voltage_ = input * 0.000125;
        break;
      case GAIN_TWO:
        calculated_voltage_ = input * 0.0000625;
        break;
      case GAIN_FOUR:
        calculated_voltage_ = input * 0.00003125;
        break;
      case GAIN_EIGHT:
        calculated_voltage_ = input * 0.000015625;
        break;
      case GAIN_SIXTEEN:
        calculated_voltage_ = input * 0.0000078125;
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
  app.onRepeat(ADS1x15RawValue<T_ads_1x15>::read_delay_, [this]() {
    ADS1x15RawValue<T_ads_1x15>::read_raw_value();
    calculate_voltage(ADS1x15RawValue<T_ads_1x15>::raw_value_);
    this->emit(calculated_voltage_);
  });
}

// define all possible instances of an ADS1x15Voltage
template class ADS1x15Voltage<ADS1015, ADS1015chip>;
template class ADS1x15Voltage<ADS1115, ADS1115chip>;
