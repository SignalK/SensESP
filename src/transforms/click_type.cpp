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

    debugD("ClickType received PRESS on click count %d (millis: %ld, last release: %ld ms ago)", click_count, millis(), (long)release_duration);

    if (click_count == 0) {
      // This is a new, isolated "click" that we have not yet processed.
      click_count++;
      press_duration = 0;
    }
    else {
      // One or more presses is already in progress...

      if (press_duration > ultra_long_click_delay) {
          // The button down is the second one reported in a row without a button release
          // and the press is now long enough to qualify as an "ultra long click"
          on_ultra_long_click();
      }
      else if (release_duration <= double_click_interval) {
        // This is the start of a second click to come in prior to the expiration of
        // the double_click_interval.  Remove any "SingleClick" report that may
        // have been queued up....
        if (delayed_click_report != NULL) {
            delayed_click_report->remove();
            delayed_click_report = NULL;
            debugD("ClickType press is double click. Removed queued SingleClick report");
        }
        click_count++;
      }

    }

    this->emit(ClickTypes::ButtonPress);

}


void ClickType::on_button_release() {

     debugD("ClickType received UNPRESS for click count %d (millis: %ld, press duration: %ld ms)", click_count, millis(), (long)press_duration);

     if (click_count > 0) {
        // This is the "release" of a click we are tracking...
        if (press_duration >= this->ultra_long_click_delay) {
            on_ultra_long_click();
            this->on_click_completed();
        }
        else if (press_duration >= this->long_click_delay) {
            debugD("ClickType detected LongSingleClick (millis: %ld, press duration %ld ms)", millis(), (long)press_duration);
            this->emitDelayed(ClickTypes::LongSingleClick);
            this->on_click_completed();
        }
        else if (this->click_count > 1) {
          // We have just ended a double click.  Sent it immediately...
          debugD("ClickType detected DoubleClick (millis: %ld, press duration %ld ms)", millis(), (long)press_duration);
          this->emitDelayed(ClickTypes::DoubleClick);
          this->on_click_completed();
        }
        else {
          // This is the end of a potential single click.  Queue up the send of a SingleClick report,
          // but delay it in case another click comes in prior to the double_click_interval, which would
          // turn this click into a DoubleClick
          unsigned long time_of_event = millis();
          long pd = (long)press_duration;
          delayed_click_report = app.onDelay(double_click_interval+20, [this, pd, time_of_event]() {
              debugD("ClickType detected SingleClick (millis: %ld, queue time: %ld, press duration %ld ms)", millis(), time_of_event, pd);
              this->emit(ClickTypes::SingleClick);
              this->on_click_completed();
          });
        }

        release_duration = 0;
        this->emit(ClickTypes::ButtonRelease);

     }
     else {
       // A press release with no initial press should happen only when
       // an UltraLongClick has already been sent, or the producer
       // is feeding us weird values...
       release_duration = 0;
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
    press_duration = 0;
    release_duration = 0;
}


void ClickType::on_ultra_long_click() {
    debugD("ClickType detected UltraLongSingleClick (millis: %ld, press duration %ld ms)", millis(), (long)press_duration);
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
