#include <Arduino.h>

#ifdef ESP8266
#include <SoftwareSerial.h>
#endif

#include "sensesp_app.h"
#include "wiring_helpers.h"

ReactESP app([]() {
  sensesp_app = new SensESPApp();

#ifndef SERIAL_DEBUG_DISABLED
  SetupSerialDebug(115200);
#endif

  // Software serial port is used for receiving NMEA data on ESP8266
  // ESP8266 pins are specified as DX
  // ESP32 pins are specified as just the X in GPIOX
#ifdef ESP8266
  uint8_t pin = D7;
  SoftwareSerial* serial = new SoftwareSerial(pin, -1);
  serial->begin(38400, SWSERIAL_8N1);
#elif defined(ESP32)
  uint8_t pin = 4;
  HardwareSerial* serial = &Serial1;
  serial->begin(38400, SERIAL_8N1, pin, -1);
#endif

  setup_gps(serial);

  sensesp_app->enable();
});
