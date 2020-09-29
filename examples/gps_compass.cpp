#include <Arduino.h>

#include "SoftwareSerial.h"
#include "sensesp_app.h"
#include "wiring_helpers.h"

ReactESP app([]() {
  sensesp_app = new SensESPApp();

  // Hardware serial port is reserved for serial terminal

  Serial.begin(115200);
  Serial.available();

  // A small arbitrary delay is required to let the
  // serial port catch up
  delay(100);
  Debug.setSerialEnabled(true);

  // Software serial port is used for receiving NMEA data
  // ESP8266 pins are specified as DX
  // ESP32 pins are specified as just the X in GPIOX
  SoftwareSerial* sw_serial = new SoftwareSerial(D7, -1);
  sw_serial->begin(38400, SWSERIAL_8N1);

  setup_gps(sw_serial);

  sensesp_app->enable();
});
