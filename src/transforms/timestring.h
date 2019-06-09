#ifndef _timestring_H_
#define _timestring_H_

#include <ctime>

#include "transform.h"
#include "system/valueconsumer.h"

class TimeString : public ValueConsumer<time_t>, public StringTransform {
 public:
  TimeString(String sk_path, String config_path="");
  virtual void set_input(time_t input, uint8_t inputChannel = 0) override final;
  String as_signalK() override final;
  virtual JsonObject& get_configuration(JsonBuffer& buf) override final;
  virtual bool set_configuration(const JsonObject& config) override final;
  virtual String get_config_schema() override;
};

#endif
