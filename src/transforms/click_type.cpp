#include "transforms/click_type.h"
#include "ReactESP.h"

ClickType::ClickType(String config_path, long long_click_delay, long double_click_interval, long ultra_long_click_delay) :
   Transform<bool, ClickTypes>(config_path),
    click_count{0},
    long_click_delay{long_click_delay},
    ultra_long_click_delay{ultra_long_click_delay},
    double_click_interval{double_click_interval},
    delayed_click_report{NULL} {
   load_configuration();
}



void ClickType::set_input(bool input, uint8_t inputChannel) {

  if (input) {
    on_button_press();
  }
  else {
    on_button_release();
  }
}

bool ClickType::is_click(ClickTypes value) {
   return (value != ClickTypes::ButtonPress && value != ClickTypes::ButtonRelease);
}


void ClickType::on_button_press() {

    debugD("ClickType received PRESS (millis: %ld, last press interval: %ld)", millis(), (long)press_released);

    if (click_count == 0) {
      // This is a new, isolated "click" that we have not yet processed.
      click_count++;
      press_started = 0;
    }
    else {
      // One or more presses is already in progress...

      if (press_started > ultra_long_click_delay) {
          // The button down is the second one reported in a row without a button release
          // and the press is now long enough to qualify as an "ultra long click"
          on_ultra_long_click("PRESS");
      }
      else if (press_released <= double_click_interval) {
        // This is the start of a second click to come in prior to the expiration of
        // the double_click_interval.  Remove any "SingleClick" report that may
        // have been queued up....
        if (delayed_click_report != NULL) {
            delayed_click_report->remove();
            delayed_click_report = NULL;
            debugD("ClickType received PRESS: double click detected. Removed queued SingleClick");
        }
      }

    }

    this->emit(ClickTypes::ButtonPress);

}


void ClickType::on_button_release() {

     if (click_count > 0) {
        // This is the "release" of a click we are tracking...
        if (press_started >= this->ultra_long_click_delay) {
            on_ultra_long_click("UNPRESSED");
            this->on_click_completed();
        }
        else if (press_started >= this->long_click_delay) {
            debugD("ClickType UNPRESSED with LongSingleClick (millis: %ld, press interval %ld)", millis(), (long)press_started);
            this->emitDelayed(ClickTypes::LongSingleClick);
            this->on_click_completed();
        }
        else if (this->click_count > 1) {
          // We have just ended a double click.  Sent it immediately...
          debugD("ClickType UNPRESSED with DoubleClick (millis: %ld, press interval %ld)", millis(), (long)press_started);
          this->emitDelayed(ClickTypes::DoubleClick);
          this->on_click_completed();
        }
        else {
          // This is the end of a potential single click.  Queue up the send of a SingleClick report,
          // but delay it in case another click comes in prior to the double_click_interval, which would
          // turn this click into a DoubleClick
          unsigned long time_of_event = millis();
          long press_interval = (long)press_started;
          delayed_click_report = app.onDelay(double_click_interval+20, [this, press_interval, time_of_event]() {
              debugD("ClickType UNPRESSED with SingleClick (millis: %ld, queue time: %ld, press interval %ld)", millis(), time_of_event, press_interval);
              this->emit(ClickTypes::SingleClick);
              this->on_click_completed();
          });
        }

        press_released = 0;
        this->emit(ClickTypes::ButtonRelease);

     }
     else {
       // A press release with no initial press should happen only when
       // an UltraLongClick has already been sent, or the producer
       // is feeding us weird values...
       press_released = 0;
       debugW("ClickType detected UNPRESS with no pending PRESS (millis=%ld)", millis());
     }
  
}



void ClickType::emitDelayed(ClickTypes value) {
   app.onDelay(5, [this, value]() {
     this->emit(value);
   });
}


void ClickType::on_click_completed() {
    this->click_count = 0;
    delayed_click_report = NULL;
    press_started = 0;
    press_released = 0;
}


void ClickType::on_ultra_long_click(const char* keyType) {
    debugD("ClickType %s with UltraLongSingleClick (millis: %ld, press interval %ld)", keyType, millis(), millis() - press_started);
    this->emitDelayed(ClickTypes::UltraLongSingleClick);
    on_click_completed();
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
