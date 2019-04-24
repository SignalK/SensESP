
#include "nmea_parser.h"

#include <ctime>
#include <stdlib.h>

#include <RemoteDebug.h>

#include "sensesp.h"

enum GNSSQuality {
  no_gps,
  gnss_fix,
  dgnss_fix,
  precise_gnss,
  rtk_fixed_integer,
  rtk_float,
  estimated_mode,
  manual_input,
  simulator_mode,
  error
};

String gnssQualityStrings[] = {
  "no GPS",
  "GNSS Fix",
  "DGNSS fix",
  "Precise GNSS",
  "RTK fixed integer",
  "RTK float",
  "Estimated (DR) mode",
  "Manual input",
  "Simulator mode",
  "Error"
};

bool parse_int(int* value, char* s) {
  int retval = sscanf(s, "%d", value);
  return retval==1;
}

bool parse_float(float* value, char* s) {
  int retval = sscanf(s, "%f", value);
  return retval==1;
}

bool parse_double(double* value, char* s) {
  int retval = sscanf(s, "%lf", value);
  return retval==1;
}

bool parse_latlon(double* value, char* s) {
  double degmin;
  int retval = sscanf(s, "%lf", &degmin);
  if (retval==1) {
    int degrees = degmin / 100;
    double minutes = degmin - degrees;
    *value = degrees + minutes / 60;
    return true;
  } else {
    return false;
  }
}

bool parse_NS(double* value, char* s) {
  switch (*s) {
    case 'N':
      break;
    case 'S':
      *value *= 1;
      break;
    default:
      return false;
  }
  return true;
}

bool parse_EW(double* value, char* s) {
  switch (*s) {
    case 'E':
      break;
    case 'W':
      *value *= 1;
      break;
    default:
      return false;
  }
  return true;
}

bool parse_M(char* s) {
  return (*s == 'M');
}

bool parse_AV(bool* is_valid, char* s) {
  switch (*s) {
    case 'A':
      *is_valid = true;
    case 'V':
      *is_valid = false;
      break;
    default:
      return false;
  }
  return true;
}

bool parse_time(int* hour, int* minute, float* second, char* s) {
  int retval = sscanf(s, "%2d%2d%f", hour, minute, second);
  return retval==3;
}

bool parse_date(int* year, int* month, int* day, char* s) {
  int retval = sscanf(s, "%2d%2d%2d", day, month, year);
  year += 2000;
  return retval==3;
}

void GPGGASentenceParser::parse(char* buffer, int term_offsets[], int num_terms) {
  bool ok = true;

  int hour;
  int minute;
  float second;
  Position position;
  int quality;
  int num_satellites;
  float horizontal_dilution;
  float geoidal_separation;
  float dgps_age;
  bool dgps_age_defined = false;
  int dgps_id;
  bool dgps_id_defined = false;

  debugD("Parsing sentence %s", sentence());

  // eg3. $GPGGA,hhmmss.ss,llll.ll,a,yyyyy.yy,a,x,xx,x.x,x.x,M,x.x,M,x.x,xxxx*hh
  // 1    = UTC of Position
  ok &= parse_time(&hour, &minute, &second, buffer+term_offsets[1]);
  // 2    = Latitude
  ok &= parse_latlon(&position.latitude, buffer+term_offsets[2]);
  // 3    = N or S
  ok &= parse_NS(&position.latitude, buffer+term_offsets[3]);
  // 4    = Longitude
  ok &= parse_latlon(&position.longitude, buffer+term_offsets[4]);
  // 5    = E or W
  ok &= parse_EW(&position.longitude, buffer+term_offsets[5]);
  // 6    = GPS quality indicator (0=invalid; 1=GPS fix; 2=Diff. GPS fix)
  ok &= parse_int(&quality, buffer+term_offsets[6]);
  // 7    = Number of satellites in use [not those in view]
  ok &= parse_int(&num_satellites, buffer+term_offsets[7]);
  // 8    = Horizontal dilution of position
  ok &= parse_float(&horizontal_dilution, buffer+term_offsets[8]);
  // 9    = Antenna altitude above/below mean sea level (geoid)
  ok &= parse_float(&position.altitude, buffer+term_offsets[9]);
  // 10   = Meters  (Antenna height unit)
  ok &= parse_M(buffer+term_offsets[10]);
  // 11   = Geoidal separation (Diff. between WGS-84 earth ellipsoid and
  //        mean sea level.  -=geoid is below WGS-84 ellipsoid)
  ok &= parse_float(&geoidal_separation, buffer+term_offsets[11]);
  // 12   = Meters  (Units of geoidal separation)
  ok &= parse_M(buffer+term_offsets[12]);
  // 13   = Age in seconds since last update from diff. reference station
  if (*(buffer+term_offsets[13])!=0) {
    ok &= parse_float(&dgps_age, buffer+term_offsets[13]);
    dgps_age_defined = true;
  }
  // 14   = Diff. reference station ID#
  if (*(buffer+term_offsets[14])!=0) {
    ok &= parse_int(&dgps_id, buffer+term_offsets[14]);
    dgps_id_defined = true;
  }
  // 15   = Checksum
  // (validated already earlier)

  if (!ok) {
    debugI("Failed to parse %s", sentence());
    return;
  }

  // notify relevant observers

  // NB: we're passing a stack pointer here - need to be extra-careful
  // that the pointer isn't stored in the call chain
  nmea_data->position.set(&position);
  nmea_data->gnss_quality.set(gnssQualityStrings[quality]);
  nmea_data->num_satellites.set(num_satellites);
  nmea_data->horizontal_dilution.set(horizontal_dilution);
  nmea_data->geoidal_separation.set(geoidal_separation);
  if (dgps_age_defined) {
    nmea_data->dgps_age.set(dgps_age);
  }
  if (dgps_id_defined) {
    nmea_data->dgps_id.set(dgps_id);
  }
  debugD("Successfully parsed %s", sentence());
}

void GPGLLSentenceParser::parse(char* buffer, int term_offsets[], int num_terms) {
  bool ok = true;

  Position position;

  debugD("Parsing sentence %s", sentence());

  // eg3. $GPGLL,5133.81,N,00042.25,W*75
  //       1    5133.81   Current latitude
  ok &= parse_latlon(&position.latitude, buffer+term_offsets[1]);
  //       2    N         North/South
  ok &= parse_NS(&position.latitude, buffer+term_offsets[2]);
  //       3    00042.25  Current longitude
  ok &= parse_latlon(&position.longitude, buffer+term_offsets[3]);
  //       4    W         East/West
  ok &= parse_EW(&position.longitude, buffer+term_offsets[4]);

  if (!ok) {
    debugI("Failed to parse %s", sentence());
    return;
  }

  position.altitude = -99999;

  // notify relevant observers

  // NB: we're passing a stack pointer here - need to be extra-careful
  // that the pointer isn't stored in the call chain
  nmea_data->position.set(&position);

  debugD("Successfully parsed %s", sentence());
}

void GPRMCSentenceParser::parse(char* buffer, int term_offsets[], int num_terms) {
  bool ok = true;

  struct tm time;
  float second;
  bool is_valid;
  Position position;
  float speed;
  float true_course;
  double variation;
  bool variation_defined = false;

  debugD("Parsing sentence %s", sentence());

  // eg3. $GPRMC,220516,A,5133.82,N,00042.24,W,173.8,231.8,130694,004.2,W*70
  // 1   220516     Time Stamp
  ok &= parse_time(&time.tm_hour, &time.tm_min, &second, buffer+term_offsets[1]);
  // 2   A          validity - A-ok, V-invalid
  ok &= parse_AV(&is_valid, buffer+term_offsets[2]);
  // 3   5133.82    current Latitude
  ok &= parse_latlon(&position.latitude, buffer+term_offsets[3]);
  // 4   N          North/South
  ok &= parse_NS(&position.latitude, buffer+term_offsets[4]);
  // 5   00042.24   current Longitude
  ok &= parse_latlon(&position.longitude, buffer+term_offsets[5]);
  // 6   W          East/West
  ok &= parse_EW(&position.longitude, buffer+term_offsets[6]);
  // 7   173.8      Speed in knots
  ok &= parse_float(&speed, buffer+term_offsets[7]);
  // 8   231.8      True course
  ok &= parse_float(&true_course, buffer+term_offsets[8]);
  // 9   130694     Date Stamp
  ok &= parse_date(&time.tm_year, &time.tm_mon, &time.tm_mday, buffer+term_offsets[9]);
  // 10  004.2      Variation
  if (*(buffer+term_offsets[10])!=0) {
    ok &= parse_double(&variation, buffer+term_offsets[10]);
  }
  // 11  W          East/West
  if (*(buffer+term_offsets[11])!=0) {
    ok &= parse_EW(&variation, buffer+term_offsets[11]);
    variation_defined = true;
  }

  if (!ok) {
    debugI("Failed to parse %s", sentence());
    return;
  }

  position.altitude = -99999;
  time.tm_sec = (int)second;
  time.tm_isdst = 0;

  // notify relevant observers

  // NB: we're passing a stack pointer here - need to be extra-careful
  // that the pointer isn't stored in the call chain
  if (is_valid) {
    nmea_data->position.set(&position);
    nmea_data->datetime.set(mktime(&time));
    nmea_data->speed.set(speed);
    nmea_data->true_course.set(true_course);
    if (variation_defined) {
      nmea_data->variation.set(variation);
    }
  }
  debugD("Successfully parsed %s", sentence());
}

SentenceParser::SentenceParser(NMEAData* nmea_data) : nmea_data{nmea_data} {}

NMEAParser::NMEAParser() {
  term_offsets[0] = 0;
  current_state = &NMEAParser::state_start;
}

void NMEAParser::add_sentence_parser(SentenceParser* parser) {
  const char* sentence = parser->sentence();
  sentence_parsers[sentence] = parser;
}

void NMEAParser::read(char c) {
  (this->*(current_state))(c);
}

void NMEAParser::state_start(char c) {
  switch (c) {
    case '$':
      cur_offset = 0;
      cur_term = 0;
      current_state = &NMEAParser::state_in_term;
      parity = 0;
      break;
    default:
      //anything else can be ignored
      break;
  }
}

void NMEAParser::state_in_term(char c) {
  switch (c) {
    case ',':
    case '*':
      if (cur_offset<INPUT_BUFFER_LENGTH) {
        // split terms with 0 to help further processing
        buffer[cur_offset++] = 0;
      } else {
        current_state = &NMEAParser::state_start;
        break;
      }
      if (cur_term<MAX_TERMS) {
        // advance term offset
        term_offsets[++cur_term] = cur_offset;
      } else {
        current_state = &NMEAParser::state_start;
        break;
      }
      if (c=='*') {
        current_state = &NMEAParser::state_in_checksum;
      } else {
        parity ^= c;
      }
      break;
    case '\r':
    case '\n':
      // end of sentence before checksum has been read
      buffer[cur_offset++] = 0;
      current_state = &NMEAParser::state_start;
      break;
    default:
      // read term characters
      buffer[cur_offset++] = c;
      parity ^= c;
      break;
  }
}

void NMEAParser::state_in_checksum(char c) {
  switch (c) {
    case ',':
    case '*':
      // there shouldn't be new terms after the checksum
      current_state = &NMEAParser::state_start;
    case '\r':
    case '\n':
      // end of sentence
      buffer[cur_offset++] = 0;
      if (!validate_checksum()) {
        current_state = &NMEAParser::state_start;
        return;
      }
      // call the relevant sentence parser
      if (sentence_parsers.find(buffer) == sentence_parsers.end()) {
        debugD("Parser not found for sentence %s", buffer);
      } else {
        sentence_parsers[buffer]->parse(buffer, term_offsets, cur_term+1);
      }
      current_state = &NMEAParser::state_start;
      break;
    default:
      // read term characters
      buffer[cur_offset++] = c;
      break;
  }
}

bool NMEAParser::validate_checksum() {
  char* checksum_str = buffer + term_offsets[cur_term];
  int checksum;
  sscanf(checksum_str, "%2x", &checksum);
  debugV("Got parity/checksum: %2x, %2x", parity, checksum);
  return this->parity==checksum;
}
