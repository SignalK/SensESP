#ifndef _button_H_
#define _button_H_

#include "sensors/digital_input.h"
#include "system/valueproducer.h"

/**
 * Button represents a switch or button connected to a digital input
 * pin. Button emits a value only when the state of the button changes
 * (i.e. when the button changes from not pressed to pressed, and vice versa).
 * However, if the button is pressed and held longer than repeat_start_interval 
 * milliseconds, the press will be emitted once again, and then again every 
 * repeat_interval milliseconds until the press is released. To
 * disable this feature, set repeat_start_interval to -1.
 */
class Button : public DigitalInput, public BooleanProducer {

    public:
        Button(uint8_t pin, String configPath = "", int repeat_start_interval = 1500, int repeat_interval = 250, int pressed_state = HIGH);

        virtual void enable() override;
        virtual void get_configuration(JsonObject& doc) override;
        virtual bool set_configuration(const JsonObject& config) override;
        virtual String get_config_schema() override;
    protected:
       int repeat_start_interval;
       int repeat_interval;
       long last_press_sent;
       bool pushed;
       bool repeating;
       int pressed_state;
};


#endif
