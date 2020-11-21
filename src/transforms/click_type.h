#ifndef _click_types_H_
#define _click_types_H_

#include "transforms/transform.h"


/**
 * ClickTypes defines the types of clicks a ClickType transform can detect
 */
enum class ClickTypes { SingleClick, LongSingleClick, UltraLongSingleClick, DoubleClick };


/**
 * ClickType adds a time element to button presses. It measures the time between
 * a button press and its release and will emit its value as an encoded version
 * of the button press. 
 */
class ClickType : public Transform<bool, ClickTypes> {

    public:

      ClickType(String config_path, int long_click_delay = 1300, int double_click_interval = 350, int ultra_long_click_delay = 5000);
      virtual void set_input(bool input, uint8_t input_channel = 0) override;
      virtual void get_configuration(JsonObject& doc) override;
      virtual bool set_configuration(const JsonObject& config) override;
      virtual String get_config_schema() override;

    protected:
      long press_started;
      long press_released;
      int click_count;
      int long_click_delay;
      int double_click_interval;
      int ultra_long_click_delay;
      DelayReaction* queued_report;

      void pressCompleted();

      void onUltraLongClick(const char* keyType);
};

#endif
