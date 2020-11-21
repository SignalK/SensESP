#ifndef _smart_switch_controller_h
#define _smart_switch_controller_h

#include "sensors/smart_switch.h"
#include "system/valueconsumer.h"
#include "transforms/click_type.h"

/**
 * SmartSwitchController is a high level transform designed specifically to
 * control a SmartSwitch. The controller accepts inputs from a generic boolean
 * producer (usually a SignalK listener), as well as String and ClickType transforms.
 * The latter allows a manual button to control the SmartSwitch as well as
 * add special behaviors to the sensor application. In particular, an ultra
 * long press of the button will cause the MCU to reboot.
 * <p>A SmartSwitchController is assumed to be a toggle switch. Normal
 * incoming click types will toggle the output of this producer on and off.
 * The incoming boolean values, on the other hand, are assumed to be
 * state setting commands (i.e. turn on or off), so those are used
 * verbatim and forwarded. Incoming string values can be one of 
 * many various "truth" values (such as "on" or "off"). Strings
 * that are valid truth values (e.g. "on", "true", "1", "off", "false", etc)
 * will set the switch accordingly. Strings that are NOT valid truth values
 * will cause the switch state to toggle. Thus, incoming strings like
 * "toggle" or "default" will toggle the current state and output the results.
 * @see SmartSwitch
 * @see TextToTruth
 */
class SmartSwitchController : public BooleanTransform,
                              public ValueConsumer<ClickTypes>,
                              public ValueConsumer<String> {

     public:
       void set_input(bool new_value, uint8_t input_channel = 0) override;
       void set_input(String new_value, uint8_t input_channel = 0) override;
       void set_input(ClickTypes new_value, uint8_t input_channel = 0) override;
};

#endif