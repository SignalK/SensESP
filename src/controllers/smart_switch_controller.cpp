#include "smart_switch_controller.h"
#include "transforms/truth_text.h"

void SmartSwitchController::set_input(bool new_value, uint8_t input_channel) {
   is_on = new_value;
   this->emit(is_on);
}


void SmartSwitchController::set_input(ClickTypes new_value, uint8_t input_channel) {

    if (new_value == ClickTypes::UltraLongSingleClick) {
        // Long clicks reboot the system...
        ESP.restart();
        return;
    }

    // All other click types toggle the current state...
    is_on = !is_on;
    this->emit(is_on);
}

void SmartSwitchController::set_input(String new_value, uint8_t input_channel) {

    if (is_valid_true(new_value)) {
        is_on = true;
    }
    else if (is_valid_false(new_value)) {
        is_on = false;      
    }
    else {
        // All other values simply toggle...
        is_on = !is_on;

    }
    this->emit(is_on);
}
