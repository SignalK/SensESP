#include "debounce.h"

Debounce::Debounce(int msMinDelay, String sk_path, String config_id, String schema) :
    OneToOneBooleanTransform(sk_path, config_id, schema), msMinDelay{msMinDelay} {

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
