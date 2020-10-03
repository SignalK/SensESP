#include "debounce.h"

Debounce::Debounce(int ms_min_delay, String config_path) :
    BooleanTransform(config_path), ms_min_delay{ms_min_delay} {
    last_time = millis();
}


void Debounce::set_input(bool newValue, uint8_t inputChannel) {

    int elapsed = millis() - last_time;
    if (newValue != output || elapsed > ms_min_delay) {
        output = newValue;
        last_time = millis();
        notify();
    }
}
