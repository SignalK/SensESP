#ifndef _digital_output_H
#define _digital_output_H

#include "system/observable.h"
#include "system/valueconsumer.h"
#include <ArduinoJson.h>

class DigitalOutput : public BooleanConsumer
{
    public: 
        DigitalOutput(int pin);
        void set_input(bool newValue, uint8_t inputChannel = 0) override;
    private:
        int pinNumber;
};

#endif