#ifndef _button_H_
#define _button_H_

#include "sensors/digital_input.h"
#include "system/valueproducer.h"

/**
 * Button represents a switch or button connected to a digital input
 * pin. Button emits a value only when the state of the button changes
 * unless the button is pressed longer than repeat_start_interval.
 * If the button is pressed longer than repeat_start_interval, a 
 * button repeat is sent every repeat_interval milliseconds. To
 * disable this feature, set repeat_start_interval to -1.
 */
class Button : public DigitalInput, public BooleanProducer {

    public:
        Button(uint8_t pin, int pin_mode = INPUT, int repeat_start_interval = 1500, int repeat_interval = 250);

        virtual void enable() override;
        virtual void get_configuration(JsonObject& doc) override;
        virtual bool set_configuration(const JsonObject& config) override;
        virtual String get_config_schema() override;
    protected:
       int repeat_start_interval;
       int repeat_interval;
       unsigned long last_press_sent;
       bool pushed;
       bool repeating;
};


#endif
