#include "debounce.h"

Debounce::Debounce(int msMinDelay, String config_path) :
    BooleanTransform(config_path), msMinDelay{msMinDelay} {
    className = "Debounce";
    lastTime = millis();
}


void Debounce::set_input(bool newValue, uint8_t inputChannel) {

    int elapsed = millis() - lastTime;
    if (newValue != output || elapsed > msMinDelay) {
        output = newValue;
        lastTime = millis();
        notify();
    }
}
