#include "transforms/click_type.h"
#include "ReactESP.h"

ClickType::ClickType(String config_path, long long_click_delay, long double_click_interval, long ultra_long_click_delay) :
   Transform<bool, ClickTypes>(config_path),
    press_started{-1},
    press_released{0},
    click_count{0},
    long_click_delay{long_click_delay},
    double_click_interval{double_click_interval},
    ultra_long_click_delay{ultra_long_click_delay},
    queued_report{NULL} {
   load_configuration();      
}


void ClickType::set_input(bool input, uint8_t inputChannel) {
  if (input) {
     // Button press has started
     if (press_started == -1) {
        // This is a change in status that we have not yet processed. Process it,
        // as long as it doesn't come quickly after the last button release...
        long unpress_interval = millis() - press_released;
        press_started = millis();
        if (unpress_interval <= double_click_interval) {
          if (queued_report != NULL) {
              queued_report->remove();
              queued_report = NULL;
              debugD("ClickType received PRESS: double click detected. Removed queued SingleClick");
          }
          click_count++;
        }
        else {
          debugD("ClickType received PRESS (millis: %ld, last press interval: %ld)", millis(), unpress_interval);
        }
     }
     else if ((long)(millis() - press_started) > ultra_long_click_delay) {
        on_ultra_long_click("PRESS");
     }
     else {
       debugW("ClickType ignoring PRESS while PRESS already started (millis=%ld)", millis());
     }
  }
  else {
     // Button has been released
     if (press_started != -1) {
        // Finish processing the button press...
        long press_interval = millis() - press_started;
        press_started = -1;
        press_released = millis();
        if (press_interval >= this->ultra_long_click_delay) {
            on_ultra_long_click("UNPRESSED");
            this->press_completed();
        }
        else if (press_interval >= this->long_click_delay) {
            debugD("ClickType UNPRESSED with LongSingleClick (millis: %ld, press interval %ld)", millis(), press_interval);
            this->emit(ClickTypes::LongSingleClick);
            this->press_completed();
        }
        else if (this->click_count >= 1) {
          // We have just ended a double click.  Sent it immediately...
          debugD("ClickType UNPRESSED with DoubleClick (millis: %ld, press interval %ld)", millis(), press_interval);
          this->emit(ClickTypes::DoubleClick);
          this->press_completed();
        }
        else {
          // This is the end of the first normal click.  Queue up a send of a single click,
          // but delay it in case another click comes in prior to the double_click_interval...
          long ms = millis();
          queued_report = app.onDelay(double_click_interval+20, [this, press_interval, ms]() {
              debugD("ClickType UNPRESSED with SingleClick (millis: %ld, queue time: %ld, press interval %ld)", millis(), ms, press_interval);
              this->emit(ClickTypes::SingleClick);
              this->press_completed();
          });
        }
     }
     else {
       press_released = millis();
       debugW("ClickType detected UNPRESS with no pending PRESS (millis=%ld)", millis());
     }
  }
}


void ClickType::press_completed() {
    this->click_count = 0;
    queued_report = NULL;
    press_started = -1;
    press_released = 0;
}

void ClickType::on_ultra_long_click(const char* keyType) {
    debugD("ClickType %s with UltraLongSingleClick (millis: %ld, press interval %ld)", keyType, millis(), millis() - press_started);
    this->emit(ClickTypes::UltraLongSingleClick);
}

void ClickType::get_configuration(JsonObject& root) {
  root["long_click_delay"] = long_click_delay;
  root["ultra_long_click_delay"] = ultra_long_click_delay;
  root["double_click_interval"] = double_click_interval;
}



static const char SCHEMA[] PROGMEM = R"({
    "type": "object",
    "properties": {
        "long_click_delay": { "title": "Long click milliseconds", "type": "integer" },
        "ultra_long_click_delay": { "title": "Ultra long click milliseconds", "type": "integer" },
        "double_click_interval": { "title": "Max millisecond interval between double clicks", "type" : "integer" }
    }
  })";


String ClickType::get_config_schema() { return FPSTR(SCHEMA); }

bool ClickType::set_configuration(const JsonObject& config) {
  String expected[] = {"long_click_delay", "ultra_long_click_delay", "double_click_interval"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  long_click_delay = config["long_click_delay"];
  ultra_long_click_delay = config["ultra_long_click_delay"];
  double_click_interval = config["double_click_interval"];
  return true;
}
