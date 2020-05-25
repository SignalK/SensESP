// ads1x15_volt_meter.cpp example

#include <Arduino.h>

//#define SERIAL_DEBUG_DISABLED

#define USE_LIB_WEBSOCKET true

#include "sensesp_app.h"
#include "sensors/ads1x15.h"
#include "signalk/signalk_output.h"
#include "transforms/linear.h"
#include "transforms/ads1x15_voltage.h"
#include "transforms/voltage_multiplier.h"

ReactESP app([] () {
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
  
  // Create an instance of a ADS1115 Analog-to-Digital Converter (ADC). 
  ADS1x15CHIP_t chip = ADS1115chip;
  adsGain_t gain = GAIN_TWOTHIRDS;
  ADS1115* ads1115 = new ADS1115(0x48, gain);

  // Create an instance of ADS1115value to read a specific channel of the ADC,
  // the channel on the physical chip that the input is connected to.
  uint8_t channel_12V = 0;
  uint read_delay_12V = 500;

  auto* p12V_AltVoltage = new ADS1115value(ads1115, channel_12V, read_delay_12V, "/12V_Alt/ADC read delay");

  // The output from the ADS1115 needs to be sent through some transforms to get it ready to display in Signal K:
  // - ADS1x15Voltage() takes the output from the ADS1115 and converts it back into the voltage that was read by the chip.
  // - VoltageMultiplier() reverses the effect of the physical voltage divider that was used to step the source voltage
  //   down to less than 5 volts, which is the maximum the ADC can read. The output is the original voltage.
  // - SKOutputNumber() is a specialized transport to send a float value to the Signal K server.

    p12V_AltVoltage->connectTo(new ADS1x15Voltage(chip, gain)) 
                  ->connectTo(new VoltageMultiplier(10000, 4730, "/12V_Alt/VoltMuliplier"))  // Measured ohm values of R1 and R2 in the physical voltage divider
                  ->connectTo(new SKOutputNumber("electrical.alternators.12V.voltage", "/12V_Alt/skPath"));


  // Create a second instance of ADS1115value to read from the same physical ADS1115, but from channel 1 instead of 0.
  uint8_t channel_24V = 1;
  uint read_delay_24V = 500;

  auto* p24VAltVoltage = new ADS1115value(ads1115, channel_24V, read_delay_24V, "/24V_Alt/ADC read delay");

    p24VAltVoltage->connectTo(new ADS1x15Voltage(chip, gain)) 
                  ->connectTo(new VoltageMultiplier(21800, 4690, "/24V_Alt/VoltMuliplier"))
                  ->connectTo(new SKOutputNumber("electrical.alternators.24V.voltage", "/24V_Alt/skPath"));                

 
  sensesp_app->enable();
});
