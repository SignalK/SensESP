#include "load_controller.h"

LoadController::LoadController(int relay_pin) : 
    relay_pin{relay_pin} {
    pinMode(relay_pin, OUTPUT);
}


void LoadController::set_input(bool new_value, uint8_t input_channel) {

    if (new_value != this->output) {
        // The state has changed...
        set_state(new_value);
        notify();
    }

}

void LoadController::enable() {
   set_input(false);
}

void LoadController::set_state(bool new_value) {
    this->output = new_value;
    digitalWrite(relay_pin, this->output ? HIGH : LOW);
}
