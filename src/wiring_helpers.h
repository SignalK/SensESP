#ifndef _wiring_helpers_H_
#define _wiring_helpers_H_

#include "sensesp_app.h"

// Obsolete
void setup_analog_input(String sk_path, float k = 1, float c = 0,
                        String config_path = "");

void setup_fuel_flow_meter(int inflow_pin, int return_flow_pin);

// Obsolete
void setup_rpm_meter(int input_pin);

#endif
