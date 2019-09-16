#include "gps.h"

#include <math.h>

#include "sensesp.h"

GPSInput::GPSInput(Stream* rx_stream, String config_path)
    : Sensor(config_path) {

  className = "GPSInput";
  this->rx_stream = rx_stream;

  nmea_parser.add_sentence_parser(new GPGGASentenceParser(&nmea_data));
  nmea_parser.add_sentence_parser(new GPGLLSentenceParser(&nmea_data));
  nmea_parser.add_sentence_parser(new GPRMCSentenceParser(&nmea_data));
  nmea_parser.add_sentence_parser(new PSTISentenceParser(&nmea_data));
  nmea_parser.add_sentence_parser(new PSTI030SentenceParser(&nmea_data));
  nmea_parser.add_sentence_parser(new PSTI032SentenceParser(&nmea_data));

  load_configuration();
}

void GPSInput::enable() {
  // enable reading the serial port
  app.onAvailable(*rx_stream, [this](){
    while (this->rx_stream->available()) {
      nmea_parser.handle(this->rx_stream->read());
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

