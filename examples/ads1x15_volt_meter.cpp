// ads1x15_volt_meter.cpp example

#include <Arduino.h>

//#define SERIAL_DEBUG_DISABLED

#define USE_LIB_WEBSOCKET true

#include "sensesp_app.h"
#include "sensors/ads1x15.h"
#include "signalk/signalk_output.h"
#include "transforms/linear.h"
#include "transforms/voltage_multiplier.h"

ReactESP app([]() {
#ifndef SERIAL_DEBUG_DISABLED
  SetupSerialDebug(115200);
#endif

  sensesp_app = new SensESPApp();

  // Create an instance of a ADS1115 Analog-to-Digital Converter (ADC).
  // You could use an ADS1015 (the 12-bit version of the chip) instead.
  adsGain_t gain = GAIN_TWOTHIRDS;
  ADS1115* ads1115 = new ADS1115(0x48, gain);

  // Create an instance of ADS1115Voltage to read a specific channel of the ADC,
  // (the channel on the physical chip that the input is connected to), and
  // convert it back into the voltage that was read by the ADC.
  ADS1x15Channel_t channel_12V = channel_0;
  uint read_delay_12V = 500;

  auto* alt_12v_voltage = new ADS1115Voltage(ads1115, channel_12V, read_delay_12V,
                                             "/12V_Alt/ADC read delay");

  // The output from ADS1115Voltage needs to be sent through some transforms to get
  // it into Signal K:
  // - VoltageMultiplier() reverses the effect of the physical voltage divider
  //   that was used to step the source voltage down to less than 5 volts, which is
  //   the maximum the ADC can read. The output is the original voltage.
  // - SKOutputNumber() is a specialized transport to send a float value to the
  //   Signal K server. 
  // - NOTE: there are more versions of SKOutput(), each used to send the
  //   appropriate type to Signal K: SKOutputInt, SKOutputBool, and SKOutputString.
  //
  // To find valid Signal K Paths that fits your need you look at this link:
  // https://signalk.org/specification/1.4.0/doc/vesselsBranch.html#vesselsregexpelectrical

  alt_12v_voltage->connect_to(new VoltageMultiplier(
                                  10000, 4730,                // Measured ohm values of R1 and
                                  "/12V_Alt/VoltMultiplier")) // R2 in the physical voltage divider
      ->connect_to(new SKOutputNumber("electrical.alternators.12V.voltage",
                                      "/12V_Alt/skPath"));

  
  // Create an instance of ADS1115RawValue to read from the same physical ADS1115, but
  // from channel 1 instead of 0. ADS1115RawValue does NOT convert the value into the 
  // voltage that was read - instead, it outputs an int from 0 to 32,768. ADS1115RawValue
  // is used when you don't care about the actual voltage, because you're measuring something
  // like a distance, or the percentage fullness of a tank, which is what this example is.
  ADS1x15Channel_t channel_tank_level = channel_1;
  uint read_delay_tank_level = 500;

  auto* tank_level = new ADS1115RawValue(ads1115, channel_tank_level, read_delay_tank_level,
                                         "/tank_level/read delay");

  // In this example, the ADS1115 is reading the output of a capacitive sensor in the form
  // of a long strip, attached vertically to the outside of a plastic water tank. When the
  // tank is empty, the ADS1115 reads a value of 800. When full, it reads 28,800.
  // So the range is 28,000 (28,800 - 800).
  // To convert the Raw Value output into a percentage, we need a multiplier and an offset:
  // Range / 100% = 280, so the multiplier is 1 / 280 = 0.003571429
  // The Raw Value at empty is 800: 800 x multiplier (0.003571429) = 2.8571432, so the offset needs
  // to be -2.8571432 to make the output 0 (percent) when the tank is empty.
  // The Raw Value at full is 28,800: 28,800 x multiplier (0.003571429) = 102.8571552.
  // Add the offset (-2.8571432) and you get 100.000012 (percent).
  // We use a Linear Transform to apply the multiplier and the offset, and then
  // SKOutputNumber to send the output (the percentage of tank fullness) to Signal K.

  float multiplier = 0.003571429;
  float offset = -2.8571432;

  tank_level->connect_to(new Linear(multiplier, offset, "/tank_level/linear"))
      ->connect_to(new SKOutputNumber("tanks.freshWater.currentLevel",
                                      "/tank_level/skPath", "ratio"));

  sensesp_app->enable();
});
