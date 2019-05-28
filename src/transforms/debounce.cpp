#include "debounce.h"

Debounce::Debounce(int msMinDelay, String sk_path, String config_path) :
    OneToOneBooleanTransform(sk_path, config_path), msMinDelay{msMinDelay} {

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
