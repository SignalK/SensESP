#ifndef _wiring_helpers_H_
#define _wiring_helpers_H_

#include "sensesp_app.h"
#include "devices/onewire_temperature.h"

void setup_analog_input(
    SensESPApp* seapp,
    String sk_path, float k=1, float c=0,
    String id="", String schema="");

void setup_digital_input(
    SensESPApp* seapp,
    int digital_pin,
    String sk_path, float k=1, float c=0,
    String id="", String schema="");

void setup_fuel_flow_meter(
  SensESPApp* seapp,
  int inflow_pin,
  int return_flow_pin
);

void setup_gps(SensESPApp* seapp, int serial_input_pin);

void setup_onewire_temperature(
  SensESPApp* seapp,
  DallasTemperatureSensors* dts,
  String sk_path,
  String id,
  String schema
);

void setup_rpm_meter(SensESPApp* seapp, int input_pin);


#endif
