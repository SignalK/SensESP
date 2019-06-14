#include "gps.h"

#include <math.h>

#include "sensesp.h"

GPSInput::GPSInput(int rx_pin, String config_path)
    : Sensor(config_path) {

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

