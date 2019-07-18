#include "gps.h"

#include <math.h>

#include "sensesp.h"

GPSInput::GPSInput(int reset_pin, String config_path)
    : Sensor(config_path) {

  className = "GPSInput";
  this->reset_pin = reset_pin;

  nmea_parser.add_sentence_parser(new GPGGASentenceParser(&nmea_data));
  nmea_parser.add_sentence_parser(new GPGLLSentenceParser(&nmea_data));
  nmea_parser.add_sentence_parser(new GPRMCSentenceParser(&nmea_data));
  nmea_parser.add_sentence_parser(new PSTISentenceParser(&nmea_data));
  nmea_parser.add_sentence_parser(new PSTI030SentenceParser(&nmea_data));
  nmea_parser.add_sentence_parser(new PSTI032SentenceParser(&nmea_data));

  load_configuration();
}

void GPSInput::enable() {
  Serial.begin(GPS_SERIAL_BITRATE);
  // This moves RX to pin 13 (D7)
  Serial.swap();

  // reset the GPS modules
  if (this->reset_pin) {
    pinMode(this->reset_pin, OUTPUT);
    digitalWrite(this->reset_pin, LOW);
    app.onDelay(100, [this](){
      digitalWrite(this->reset_pin, HIGH);
      pinMode(this->reset_pin, INPUT);
    });
  }

  // enable reading the serial port
  app.onAvailable(Serial, [this](){
    while (Serial.available()) {
      nmea_parser.read(Serial.read());
    }
  });

  //#ifndef DEBUG_DISABLED
  //app.onRepeat(1000, [this](){
  //  debugD("GPS characters processed: %d", gps.charsProcessed());
  //  debugD("Sentences with fix: %d", gps.sentencesWithFix());
  //  debugD("Passed checksum: %d", gps.passedChecksum());
  //  debugD("Failed checksum: %d", gps.failedChecksum());
  //});
  //#endif
}

