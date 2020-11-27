#ifndef _click_types_H_
#define _click_types_H_

#include <elapsedMillis.h>

#include "transforms/transform.h"


/**
 * ClickTypes defines the types of clicks and button presses a ClickType transform can detect
 */
enum class ClickTypes { ButtonPress, ButtonRelease, SingleClick, LongSingleClick, UltraLongSingleClick, DoubleClick };


/**
 * ClickType adds a time element to button presses. It measures the time between
 * a button press and its release and will emit its value as an encoded version
 * of the button press. 
 */
class ClickType : public Transform<bool, ClickTypes> {

    public:

      /**
       * The constructor
       * @param config_path The configuration path to use if you want the end user to be
       * able to change these values with the configuration UI. Leave as blank to 
       * disable this feature.
       * @param long_click_delay The number of milliseconds that an incoming button
       *  press must be held to differentiate between a SingleClick and a LongSingleClick.
       *  This value should be less than ultra_long_click_delay
       * @param double_click_interval The maximum number of milliseconds that can pass
       *  before two consecutive button presses are sent as two SingleClick events, or
       *  a single DoubleClick event.  If the interval is less than or equal to 
       *  double_click_interval milliseconds, a single DoubleClick event is emitted.
       * @param ultra_long_click_delay The number of milliseconds that an incoming button
       *  press must be held to register a single UltraLongSingleClick. Once a button
       *  has been held for ultra_long_click_delay milliseoncs, an UltraLongSingleClick
       *  is immediately emitted. This value should be longer than long_click_delay
       * 
       */
      ClickType(String config_path = "", long long_click_delay = 1300, long double_click_interval = 400, long ultra_long_click_delay = 5000);

      /**
       * Returns TRUE if the specified value is one of the higher level click interpretations
       * like SingleClick or UltraLongDoubleClick. It returns FALSE if the value is
       * ButtonPress or ButtonRelease
       */
      static bool is_click(ClickTypes value);

      virtual void set_input(bool input, uint8_t input_channel = 0) override;
      virtual void get_configuration(JsonObject& doc) override;
      virtual bool set_configuration(const JsonObject& config) override;
      virtual String get_config_schema() override;

    protected:
      /// A counter to specify how many clicks are currently being
      /// processed. Used to distinguish between single and double
      /// clicks.
      int click_count;


      // User Configuration variables...

      /// How many milliseconds a button is pressed to distinguish between
      /// a normal SingleClick and a LongSingleClick
      long long_click_delay;

      /// How many milliseoncs a button is pressed to distinguish between
      /// a normal SingleClick and an UltraLongSingleClick
      long ultra_long_click_delay;

      /// The maximum number of milliseconds that can pass before two
      /// clicks in a row are combined into a single DoubleClick
      long double_click_interval;


      /// Timmer to time button presses
      elapsedMillis press_started;


      /// Timer to time interval between button releases
      elapsedMillis press_released;


      /// Holds a delayed "SingleClick" report that we can pull back
      /// if the second click of a double click comes through. This
      /// value will be NULL if no click report is currently
      /// queued up.
      DelayReaction* delayed_click_report;


      /// Processes incoming values that represent a "ButonPress" event
      void on_button_press();


      /// Processes incoming value that represent a "ButtonRelease" event
      void on_button_release();


      /// Processes an ultra long click. Unlike the other click types,
      /// ultra long clicks are executed as soon as they pass the
      /// "ultra long"
      void on_ultra_long_click(const char* keyType);



      /**
       * Resets click tracking variables after a press has been released so the
       * next button press can be processed.
       */
      void on_click_completed();


      /**
       * Emits the specified value after a 5 millisecond delay. This allows
       * translated click types like SingleClick and DoubleClick to be
       * sent but delays its processing so the ClickTyes::ButtonReleased
       * can propogate through the system.
       */
      void emitDelayed(ClickTypes value);
};

#endif
