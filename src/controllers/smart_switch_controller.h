#ifndef _smart_switch_controller_h
#define _smart_switch_controller_h

#include "system/valueconsumer.h"
#include "transforms/click_type.h"

/**
 * @brief A high level transform designed to control a digital output
 * (such as a relay) via manual button presses or programatic commands. 
 * To accomplish this, the controller accepts inputs from a generic boolean
 * producer (usually a SignalK listener), as well as String and ClickType inputs.
 * The latter allows a physical button to control the load as well as
 * add special behaviors to the sensor application. In particular, an ultra
 * long press of the button will cause the MCU to reboot.
 * <p>A SmartSwitchController object behaves differently depending on the type of 
 * input it receives. If the input is a boolean or a "valid truth type" 
 * (which is a specialized type of boolean - see below), SmartSwitchController's 
 * output will be "on" if the input is "true", or "off" if the input is "false". 
 * If the input is a ClickType, or a String that's NOT a "valid truth type", 
 * SmartSwitchController's output will simply toggle the output back and forth 
 * between "on" and "off" with each input. The one exception to this is if
 * the input is a ClickType with the value ClickTypes::UltraLongSingleClick
 * which will reboot the MCU.
 * <p>Incoming String values are evaluated to see if they represent a "valid truth type". 
 * Examples include "on", "ON", "off", "true", "false", "one", "1", "123" (or any other string
 * that represents a non-zero value), etc. Case is insensitive. Any 
 * incoming String that doesn't evaluate to a "valid truth type" will be treated as 
 * a "click", and will toggle the output btween "on" and "off". 
 * @see TextToTruth 
 * @see ClickType
 */
class SmartSwitchController : public BooleanTransform,
                              public ValueConsumer<ClickTypes>,
                              public ValueConsumer<String> {

     public:
       void enable() override;
       void set_input(bool new_value, uint8_t input_channel = 0) override;
       void set_input(String new_value, uint8_t input_channel = 0) override;
       void set_input(ClickTypes new_value, uint8_t input_channel = 0) override;

     protected:
       bool is_on = false;
};

#endif