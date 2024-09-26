#include "click_type.h"

#include "ReactESP.h"
#include "sensesp_base_app.h"

namespace sensesp {

ClickType::ClickType(const String& config_path, uint16_t long_click_delay,
                     uint16_t double_click_interval,
                     uint16_t ultra_long_click_delay)
    : Transform<bool, ClickTypes>(config_path),
      click_count_{0},
      long_click_delay_{long_click_delay},
      ultra_long_click_delay_{ultra_long_click_delay},
      double_click_interval_{double_click_interval},
      delayed_click_report_{NULL} {
  load_configuration();
}

void ClickType::set(const bool& input) {
  if (input) {
    on_button_press();
  } else {
    on_button_release();
  }
}

bool ClickType::is_click(ClickTypes value) {
  return (value != ClickTypes::ButtonPress &&
          value != ClickTypes::ButtonRelease);
}

void ClickType::on_button_press() {
  ESP_LOGD(
      __FILENAME__,
      "ClickType received PRESS on click count %d (millis: %ld, last release: "
      "%ld ms ago)",
      click_count_, millis(), (long)release_duration_);

  if (click_count_ == 0) {
    // This is a new, isolated "click" that we have not yet processed.
    click_count_++;
    press_duration_ = 0;
  } else {
    // One or more presses is already in progress...

    if (press_duration_ > ultra_long_click_delay_) {
      // The button down is the second one reported in a row without a button
      // release and the press is now long enough to qualify as an "ultra long
      // click"
      on_ultra_long_click();
    } else if (release_duration_ <= double_click_interval_) {
      // This is the start of a second click to come in prior to the expiration
      // of the double_click_interval.  Remove any "SingleClick" report that may
      // have been queued up....
      if (delayed_click_report_ != NULL) {
        delayed_click_report_->remove(SensESPBaseApp::get_event_loop());
        delayed_click_report_ = NULL;
        ESP_LOGD(__FILENAME__,
                 "ClickType press is double click. Removed queued SingleClick "
                 "report");
      }
      click_count_++;
    }
  }

  this->emit(ClickTypes::ButtonPress);
}

void ClickType::on_button_release() {
  ESP_LOGD(__FILENAME__,
           "ClickType received UNPRESS for click count %d (millis: %ld, press "
           "duration: %ld ms)",
           click_count_, millis(), (long)press_duration_);

  if (click_count_ > 0) {
    // This is the "release" of a click we are tracking...
    if (press_duration_ >= this->ultra_long_click_delay_) {
      on_ultra_long_click();
      this->on_click_completed();
    } else if (press_duration_ >= this->long_click_delay_) {
      ESP_LOGD(
          __FILENAME__,
          "ClickType detected LongSingleClick (millis: %ld, press duration %ld "
          "ms)",
          millis(), (long)press_duration_);
      this->emitDelayed(ClickTypes::LongSingleClick);
      this->on_click_completed();
    } else if (this->click_count_ > 1) {
      // We have just ended a double click.  Sent it immediately...
      ESP_LOGD(
          __FILENAME__,
          "ClickType detected DoubleClick (millis: %ld, press duration %ld ms)",
          millis(), (long)press_duration_);
      this->emitDelayed(ClickTypes::DoubleClick);
      this->on_click_completed();
    } else {
      // This is the end of a potential single click.  Queue up the send of a
      // SingleClick report, but delay it in case another click comes in prior
      // to the double_click_interval, which would turn this click into a
      // DoubleClick
      uint64_t const time_of_event = millis();
      int64_t const pd = (long)press_duration_;
      delayed_click_report_ = SensESPBaseApp::get_event_loop()->onDelay(
          double_click_interval_ + 20, [this, pd, time_of_event]() {
            ESP_LOGD(
                __FILENAME__,
                "ClickType detected SingleClick (millis: %ld, queue time: %ld, "
                "press duration %ld ms)",
                millis(), static_cast<long>(time_of_event),
                static_cast<long>(pd));
            this->emit(ClickTypes::SingleClick);
            this->on_click_completed();
          });
    }

    release_duration_ = 0;
    this->emit(ClickTypes::ButtonRelease);

  } else {
    // A press release with no initial press should happen only when
    // an UltraLongClick has already been sent, or the producer
    // is feeding us weird values...
    release_duration_ = 0;
    ESP_LOGW(__FILENAME__,
             "ClickType detected UNPRESS with no pending PRESS (millis=%ld)",
             millis());
  }
}

void ClickType::emitDelayed(ClickTypes value) {
  SensESPBaseApp::get_event_loop()->onDelay(
      5, [this, value]() { this->emit(value); });
}

void ClickType::on_click_completed() {
  this->click_count_ = 0;
  delayed_click_report_ = NULL;
  press_duration_ = 0;
  release_duration_ = 0;
}

void ClickType::on_ultra_long_click() {
  ESP_LOGD(
      __FILENAME__,
      "ClickType detected UltraLongSingleClick (millis: %ld, press duration "
      "%ld ms)",
      millis(), (long)press_duration_);
  this->emitDelayed(ClickTypes::UltraLongSingleClick);
  on_click_completed();
}

void ClickType::get_configuration(JsonObject& root) {
  root["long_click_delay"] = long_click_delay_;
  root["ultra_long_click_delay"] = ultra_long_click_delay_;
  root["double_click_interval"] = double_click_interval_;
}

static const char kSchema[] = R"({
    "type": "object",
    "properties": {
        "long_click_delay": { "title": "Long click milliseconds", "type": "integer" },
        "ultra_long_click_delay": { "title": "Ultra long click milliseconds", "type": "integer" },
        "double_click_interval": { "title": "Max millisecond interval between double clicks", "type" : "integer" }
    }
  })";

String ClickType::get_config_schema() { return (kSchema); }

bool ClickType::set_configuration(const JsonObject& config) {
  String const expected[] = {"long_click_delay", "ultra_long_click_delay",
                             "double_click_interval"};
  for (auto str : expected) {
    if (!config[str].is<JsonVariant>()) {
      return false;
    }
  }
  long_click_delay_ = config["long_click_delay"];
  ultra_long_click_delay_ = config["ultra_long_click_delay"];
  double_click_interval_ = config["double_click_interval"];
  return true;
}

}  // namespace sensesp
