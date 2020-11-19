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
 * incomming clicks will toggle the output of this producer on and off.
 * The incomming boolean values, on the other hand, are assumed to be
 * state setting commands (i.e. turn on or off), so those are used
 * verbatim and forwarded. Incomming string values can be one of 
 * many various "truth" values (such as "on" or "true")
 * @see SmartSwitch
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