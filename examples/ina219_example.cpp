// INA219_example.cpp

#include <Arduino.h>

//#define SERIAL_DEBUG_DISABLED

#define USE_LIB_WEBSOCKET true

#include "sensesp_app.h"
#include "sensors/ina219.h"
#include "signalk/signalk_output.h"

ReactESP app([]() {
#ifndef SERIAL_DEBUG_DISABLED
  Serial.begin(115200);

  // A small delay and one debugI() are required so that
  // the serial output displays everything
  delay(100);
  Debug.setSerialEnabled(true);
#endif
  delay(100);
  debugI("Serial debug enabled");

  sensesp_app = new SensESPApp();

  // Create an INA219, which represents the physical sensor.
  // 0x40 is the default address. Chips can be modified to use 0x41 (shown
  // here), 0x44, or 0x45. The default volt and amp ranges are 32V and 2A
  // (cal32_2). Here, 32v and 1A is specified with cal32_1.
  auto* ina219 = new INA219(0x41, cal32_1);

  // Define the read_delay you're going to use, if other than the default of 500
  // ms.
  const uint read_delay = 1000;  // once per second

  // Create an INA219value, which is used to read a specific value from the
  // INA219, and send its output to SignalK as a number (float). This one is for
  // the bus voltage.
  auto* ina219_bus_voltage = new INA219value(ina219, bus_voltage, read_delay,
                                             "/someElectricDevice/busVoltage");

  ina219_bus_voltage->connect_to(
      new SKOutputNumber("electrical.someelectricdevice.busVoltage"));

  // Do the same for the shunt voltage.
  auto* ina219_shunt_voltage = new INA219value(
      ina219, shunt_voltage, read_delay, "/someElectricDevice/shuntVoltage");

  ina219_shunt_voltage->connect_to(
      new SKOutputNumber("electrical.someelectricdevice.shuntVoltage"));

  // Do the same for the current (amperage).
  auto* ina219_current = new INA219value(ina219, current, read_delay,
                                         "/someElectricDevice/current");

  ina219_current->connect_to(
      new SKOutputNumber("electrical.someelectricdevice.current"));

  // Do the same for the power (watts).
  auto* ina219_power =
      new INA219value(ina219, power, read_delay, "/someElectricDevice/power");

  ina219_power->connect_to(
      new SKOutputNumber("electrical.someelectricdevice.power"));

  // Do the same for the load voltage.
  auto* ina219_load_voltage = new INA219value(
      ina219, load_voltage, read_delay, "/someElectricDevice/loadVoltage");

  ina219_load_voltage->connect_to(
      new SKOutputNumber("electrical.someelectricdevice.loadVoltage"));

  sensesp_app->enable();
});
