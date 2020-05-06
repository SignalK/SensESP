// INA219_example.cpp

#include <Arduino.h>

//#define SERIAL_DEBUG_DISABLED

#define USE_LIB_WEBSOCKET true

#include "sensesp_app.h"
#include "signalk/signalk_output.h"
#include "sensors/ina219.h"

ReactESP app([] () {
  #ifndef SERIAL_DEBUG_DISABLED
  Serial.begin(115200);

  // A small arbitrary delay is required to let the
  // serial port catch up

  delay(100);
  Debug.setSerialEnabled(true);
  #endif

  // Create the SensESPApp with whatever "standard sensors" you want: noStdSensors, allStdSensors, or uptimeOnly.
  // The default is allStdSensors.
  sensesp_app = new SensESPApp(allStdSensors);

  // Create an INA219, which represents the physical sensor.
  // 0x40 is the default address. Chips can be modified to use 0x41 (shown here), 0x44, or 0x45.
  // The default volt and amp ranges are 32V and 2A (cal32_2). Here, 32v and 1A is specified with cal32_1.
  auto* pINA219 = new INA219(0x41, cal32_1);


  // Define the read_delay you're going to use, if other than the default of 500 ms.
  const uint read_delay = 1000; // once per second

  // Create an INA219value, which is used to read a specific value from the INA219, and send its output
  // to SignalK as a number (float). This one is for the bus voltage.
  auto* pINA219busVoltage = new INA219value(pINA219, bus_voltage, read_delay, "/someElectricDevice/busVoltage");
      
      pINA219busVoltage->connectTo(new SKOutputNumber("electrical.someelectricdevice.busVoltage"));

  // Do the same for the shunt voltage.
  auto* pINA219shuntVoltage = new INA219value(pINA219, shunt_voltage, read_delay, "/someElectricDevice/shuntVoltage");
      
      pINA219shuntVoltage->connectTo(new SKOutputNumber("electrical.someelectricdevice.shuntVoltage"));

  // Do the same for the current (amperage).
  auto* pINA219current = new INA219value(pINA219, current, read_delay, "/someElectricDevice/current");
      
      pINA219current->connectTo(new SKOutputNumber("electrical.someelectricdevice.current"));   

  // Do the same for the power (watts).
  auto* pINA219power = new INA219value(pINA219, power, read_delay, "/someElectricDevice/power");
      
      pINA219power->connectTo(new SKOutputNumber("electrical.someelectricdevice.power"));  

  // Do the same for the load voltage.
  auto* pINA219loadVoltage = new INA219value(pINA219, load_voltage, read_delay, "/someElectricDevice/loadVoltage");
      
      pINA219loadVoltage->connectTo(new SKOutputNumber("electrical.someelectricdevice.loadVoltage"));         


  sensesp_app->enable();
});
