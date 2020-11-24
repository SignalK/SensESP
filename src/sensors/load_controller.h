#ifndef _load_controller_H_
#define _load_controller_H_

#include "transforms/transform.h"

/**
 * LoadController is a specialized digital output controller intended to be wired to a
 * a relay or MOSFET that controls a load.  It consumes boolean values used to set 
 * the state of the switch. After setting the status of the relay, the value is then 
 * forwarded along, as this device is also a BooleanProducer.  Note that values 
 * will only be produced if an actual change occurs on the LoadController.
 * @see SmartSwitchController
 */
class LoadController : public BooleanTransform {

   public:
       /**
        * The constructor
        * @param relay_pin the digital output pin that controls the relay
        */
       LoadController(int relay_pin);

       void set_input(bool new_value, uint8_t input_channel = 0) override;
       void enable() override;

       /**
        * set_state() allows the state of the switch to be set without any
        * type of broadcast to observers. This is usually used for initialization
        * as well as resetting.
        */
       void set_state(bool new_value);

   protected:
       int relay_pin;

};

#endif