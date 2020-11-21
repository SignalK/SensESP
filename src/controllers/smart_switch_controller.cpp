#include "smart_switch_controller.h"
#include "transforms/truth_text.h"

void SmartSwitchController::set_input(bool new_value, uint8_t input_channel) {
   this->emit(new_value);
}


void SmartSwitchController::set_input(ClickTypes new_value, uint8_t input_channel) {

    if (new_value == ClickTypes::UltraLongSingleClick) {
        // Long clicks reboot the system...
        ESP.restart();
        return;
    }

    // All other clicks toggle the current state...
    this->emit(!output);
}

void SmartSwitchController::set_input(String new_value, uint8_t input_channel) {

    if (is_valid_true(new_value)) {
        this->emit(true);
    }
    else if (is_valid_false(new_value)) {
        this->emit(false);        
    }
    else {
        // All other values simply toggle...
        this->emit(!output);
    }

}