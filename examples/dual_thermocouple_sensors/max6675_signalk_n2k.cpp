// Example code to read MAX6675 thermocouple amplifiers
// and provide to signalk exhaust gas temperature and NMEA 2000

#include <Arduino.h>
#include "sensesp/signalk/signalk_output.h"
#include "sensesp/sensors/system_info.h"
#include "sensesp_app_builder.h"

// CAN bus (NMEA 2000) pins on SH-ESP32
#include <N2kMessages.h>
#include <NMEA2000_esp32.h>
#define CAN_RX_PIN GPIO_NUM_34
#define CAN_TX_PIN GPIO_NUM_32

// MAX6675
#include <max6675.h>
#define thermoDO GPIO_NUM_18
#define thermoCLK GPIO_NUM_25
#define thermo1CS GPIO_NUM_26
#define thermo2CS GPIO_NUM_27

// Serial console debugging disable
// #define SERIAL_DEBUG_DISABLED

using namespace sensesp;

tNMEA2000* nmea2000;

ReactESP app;

// MAX6675 setup
MAX6675 thermocouple0(thermoCLK, thermo1CS, thermoDO);
MAX6675 thermocouple1(thermoCLK, thermo2CS, thermoDO);

float temp0 = 0;
float temp1 = 0;

float temp0_callback() {
  temp0 = thermocouple0.readCelsius() + 273.15;
  return (temp0);
}

float temp1_callback() {
  temp1 = thermocouple1.readCelsius() + 273.15;
  return (temp1);
}

void setup() {
#ifndef SERIAL_DEBUG_DISABLED
  SetupSerialDebug(115200);
#endif

  SensESPAppBuilder builder;

  sensesp_app = (&builder)
                  // Set a custom hostname for the app.
                  ->set_hostname("egt-temp")
                  // Optionally, hard-code the WiFi and Signal K server
                  // settings. This is normally not needed.
                  ->set_wifi("XXXXXXX", "XXXXXXXX")
                  //->set_sk_server("XXXXXXXX", 3443)
                  ->get_app();

  auto* engine_0_egt_temperature = new RepeatSensor<float>(1000, temp0_callback); 
  auto* engine_1_egt_temperature = new RepeatSensor<float>(1000, temp1_callback);

  // define metadata for sensors
  auto engine_egt_temperature_metadata =
      new SKMetadata("K",                        // units
                     "Exhaust Gas Temperature",  // display name
                     "Exhaust Gas Temperature",  // description
                     "EGT",                      // short name
                     10.                         // timeout, in seconds
      );

  // connect the sensors to Signal K output paths
  engine_0_egt_temperature->connect_to(
      new SKOutput<float>("propulsion.0.exhaustTemperature",
                          engine_egt_temperature_metadata));

  engine_1_egt_temperature->connect_to(
      new SKOutput<float>("propulsion.1.exhaustTemperature",
                          engine_egt_temperature_metadata));

  // initialize the NMEA 2000 subsystem
  // instantiate the NMEA2000 object
  nmea2000 = new tNMEA2000_esp32(CAN_TX_PIN, CAN_RX_PIN);

  // Reserve enough buffer for sending all messages. This does not work on small
  // memory devices like Uno or Mega
  nmea2000->SetN2kCANSendFrameBufSize(250);
  nmea2000->SetN2kCANReceiveFrameBufSize(250);

  // Set Product information
  nmea2000->SetProductInformation(
      "7630101",  // Manufacturer's Model serial code (max 32 chars)
      103,         // Manufacturer's product code
      "SME EGT Interface",  // Manufacturer's Model ID (max 33 chars)
      "0.2 (2024-04-17)",  // Manufacturer's Software version code (max 40
                               // chars)
      "0.1 (2024-03-22)"   // Manufacturer's Model version (max 24 chars)
  );

  // Set device information
  nmea2000->SetDeviceInformation(
      7630101.01,    // Unique number. Use e.g. Serial number.
      160,  // Device function=Engine Gateway
      50,   // Device class=Propulsion
            // https://manualzz.com/doc/12647142/nmea2000-class-and-function-codes
      2012  // Manufacture code, free number taken from
            // https://ttlappalainen.github.io/NMEA2000/md_7_glossary.html#secRefMfgCodes
  );

  nmea2000->SetMode(tNMEA2000::N2km_NodeOnly, 22);
  // Disable all msg forwarding to USB (=Serial)
  nmea2000->EnableForward(false);
  nmea2000->Open();

  // No need to parse the messages at every single loop iteration; 1 ms will do
  app.onRepeat(1, []() { nmea2000->ParseMessages(); });

  // Implement the N2K PGN sending.
  engine_0_egt_temperature->connect_to(
      new LambdaConsumer<float>([](float temperature) {
        tN2kMsg N2kMsg;
        SetN2kPGN130316(N2kMsg,
                        1,                            // SID
                        0,                            // TempInstance
                        N2kts_ExhaustGasTemperature,  // TempSource
                        temperature                   // actual temperature
        );
        nmea2000->SendMsg(N2kMsg);
      }));

  engine_1_egt_temperature->connect_to(
      new LambdaConsumer<float>([](float temperature) {
        tN2kMsg N2kMsg;
        SetN2kPGN130316(N2kMsg,
                        1,                            // SID
                        1,                            // TempInstance
                        N2kts_ExhaustGasTemperature,  // TempSource
                        temperature                   // actual temperature
        );
        nmea2000->SendMsg(N2kMsg);
      }));


  sensesp_app->start();
}

// main program loop
void loop() { app.tick(); }
