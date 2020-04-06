#ifndef _wiring_helpers_H_
#define _wiring_helpers_H_

#include "sensesp_app.h"
#include "sensors/gps.h"
#include "sensors/onewire_temperature.h"

// Obsolete
void setup_analog_input(
    String sk_path, float k=1, float c=0,
    String config_path="");

void setup_fuel_flow_meter(
  int inflow_pin,
  int return_flow_pin
);

GPSInput* setup_gps(Stream* rx_stream);

//Obsolete
void setup_onewire_temperature(
  DallasTemperatureSensors* dts,
  String sk_path,
  String config_path
);

//Obsolete
void setup_rpm_meter(int input_pin);


#endif
