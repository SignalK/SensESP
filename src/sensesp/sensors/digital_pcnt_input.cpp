#include "digital_pcnt_input.h"

namespace sensesp {

DigitalInputPcntCounter::DigitalInputPcntCounter(uint8_t pin, int pin_mode,
                                                 int interrupt_type,
                                                 unsigned int read_delay,
                                                 String config_path)
    : DigitalInput(pin, pin_mode),
      Sensor(config_path),
      read_delay_(read_delay),
      interrupt_type_(interrupt_type) {
  load();

  ESP_ERROR_CHECK(configurePcnt(interrupt_type));

  event_loop()->onRepeat(read_delay_, [this]() {
    noInterrupts();
    int count = 0;
    ESP_ERROR_CHECK(pcnt_unit_get_count(pcnt_unit_, &count));
    ESP_ERROR_CHECK(pcnt_unit_clear_count(pcnt_unit_));
    output_ = count;
    interrupts();
    notify();
  });
}

DigitalInputPcntCounter::~DigitalInputPcntCounter() {
  if (pcnt_channel_) {
    ESP_ERROR_CHECK(pcnt_del_channel(pcnt_channel_));
  }
  if (pcnt_unit_) {
    ESP_ERROR_CHECK(pcnt_unit_stop(pcnt_unit_));
    ESP_ERROR_CHECK(pcnt_unit_disable(pcnt_unit_));
    ESP_ERROR_CHECK(pcnt_del_unit(pcnt_unit_));
  }
}

esp_err_t DigitalInputPcntCounter::configurePcnt(int interrupt_mode) {
  pcnt_unit_config_t unit_config = {
      .low_limit = -1,
      .high_limit = INT16_MAX,
  };

  ESP_ERROR_CHECK(pcnt_new_unit(&unit_config, &pcnt_unit_));

  pcnt_chan_config_t chan_config = {.edge_gpio_num = pin_,
                                    .level_gpio_num = -1,
                                    .flags = {.invert_edge_input = false,
                                              .invert_level_input = false,
                                              .virt_edge_io_level = 0,
                                              .virt_level_io_level = 0,
                                              .io_loop_back = false}};

  ESP_ERROR_CHECK(pcnt_new_channel(pcnt_unit_, &chan_config, &pcnt_channel_));

  pcnt_channel_edge_action_t edge_action_pos =
      (interrupt_mode == RISING || interrupt_mode == CHANGE)
          ? PCNT_CHANNEL_EDGE_ACTION_INCREASE
          : PCNT_CHANNEL_EDGE_ACTION_HOLD;
  pcnt_channel_edge_action_t edge_action_neg =
      (interrupt_mode == FALLING || interrupt_mode == CHANGE)
          ? PCNT_CHANNEL_EDGE_ACTION_INCREASE
          : PCNT_CHANNEL_EDGE_ACTION_HOLD;
  ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_channel_, edge_action_pos,
                                               edge_action_neg));

  ESP_ERROR_CHECK(pcnt_unit_enable(pcnt_unit_));
  ESP_ERROR_CHECK(pcnt_unit_clear_count(pcnt_unit_));

  return pcnt_unit_start(pcnt_unit_);
}

bool DigitalInputPcntCounter::to_json(JsonObject& root) {
  root["read_delay"] = read_delay_;
  return true;
}

bool DigitalInputPcntCounter::from_json(const JsonObject& config) {
  String const expected[] = {"read_delay"};
  for (auto str : expected) {
    if (!config[str].is<JsonVariant>()) {
      return false;
    }
  }
  read_delay_ = config["read_delay"];
  return true;
}

const String ConfigSchema(const DigitalInputPcntCounter& obj) {
  return R"###({"type":"object","properties":{"read_delay":{"title":"Read delay","type":"number","description":"The time, in milliseconds, between each read of the input"}}  })###";
}

bool ConfigRequiresRestart(const DigitalInputPcntCounter& obj) { return true; }

}  // namespace sensesp