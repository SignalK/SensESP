
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

// reconstruct the original NMEA sentence for debugging purposes
void reconstruct_nmea_sentence(
    char* sentence,
    const char* buffer,
    int term_offsets[],
    int num_terms) {

      // get the total length of the sentence
      int last_term_loc = term_offsets[num_terms-1];
      int last_term_len = strlen(buffer+term_offsets[num_terms-1]);
      // include the final \0
      int sentence_len = last_term_loc + last_term_len + 1;

      // beginning $
      sentence[0] = '$';
      // copy the buffer contents
      memcpy(sentence+1, buffer, sentence_len);

      // fill in the gaps with commas
      for (int i=1; i< num_terms-1; i++) {
        sentence[term_offsets[i]] = ',';
      }
      // the final gap has an asterisk
      sentence[term_offsets[num_terms-1]] = '*';
}

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
    double minutes = degmin - 100*degrees;
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
      break;
    case 'V':
      *is_valid = false;
      break;
    default:
      return false;
  }
  return true;
}

bool parse_PSTI030_mode(GNSSQuality* quality, char* s) {
  switch (*s) {
    case 'N':
      *quality = GNSSQuality::no_gps;
      break;
    case 'A':
      *quality = GNSSQuality::gnss_fix;
      break;
    case 'D':
      *quality = GNSSQuality::dgnss_fix;
      break;
    case 'E':
      *quality = GNSSQuality::estimated_mode;
      break;
    case 'M':
      *quality = GNSSQuality::manual_input;
      break;
    case 'S':
      *quality = GNSSQuality::simulator_mode;
      break;
    case 'F':
      *quality = GNSSQuality::rtk_float;
      break;
    case 'R':
      *quality = GNSSQuality::rtk_fixed_integer;
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
  // date expressed as C struct tm
  *year += 100;
  *month -= 1;
  return retval==3;
}

void report_success(bool ok, const char* sentence) {
  if (!ok) {
    debugI("Failed to parse %s", sentence);
    return;
  } else {
    debugD("Parsed sentence %s", sentence);
  }
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

  report_success(ok, sentence());
  if (!ok) {
    return;
  }

  // notify relevant observers

  nmea_data->position.set(position);
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
}

void GPGLLSentenceParser::parse(char* buffer, int term_offsets[], int num_terms) {
  bool ok = true;

  Position position;

  // eg3. $GPGLL,5133.81,N,00042.25,W*75
  //       1    5133.81   Current latitude
  ok &= parse_latlon(&position.latitude, buffer+term_offsets[1]);
  //       2    N         North/South
  ok &= parse_NS(&position.latitude, buffer+term_offsets[2]);
  //       3    00042.25  Current longitude
  ok &= parse_latlon(&position.longitude, buffer+term_offsets[3]);
  //       4    W         East/West
  ok &= parse_EW(&position.longitude, buffer+term_offsets[4]);

  report_success(ok, sentence());
  if (!ok) {
    return;
  }

  position.altitude = -99999;

  // notify relevant observers

  nmea_data->position.set(position);
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

  report_success(ok, sentence());
  if (!ok) {
    return;
  }

  position.altitude = -99999;
  time.tm_sec = (int)second;
  time.tm_isdst = 0;

  // notify relevant observers

  if (is_valid) {
    nmea_data->position.set(position);
    nmea_data->datetime.set(mktime(&time));
    nmea_data->speed.set(1852.*speed/3600.);
    nmea_data->true_course.set(2*PI*true_course/360.);
    if (variation_defined) {
      nmea_data->variation.set(2*PI*variation/360.);
    }
  }
}

void PSTISentenceParser::parse(
    char* buffer, int term_offsets[], int num_terms,
    std::map<String, SentenceParser*>& sentence_parsers
  ) {
  bool ok = true;
  int subsentence;

  ok &= parse_int(&subsentence, buffer+term_offsets[1]);

  report_success(ok, sentence());
  if (!ok) {
    return;
  }

  switch (subsentence) {
    case 30:
      sentence_parsers["PSTI,030"]->parse(buffer, term_offsets, num_terms);
      break;
    case 32:
      sentence_parsers["PSTI,032"]->parse(buffer, term_offsets, num_terms);
      break;
  }
}

void PSTI030SentenceParser::parse(char* buffer, int term_offsets[], int num_terms) {
  bool ok = true;

  struct tm time;
  float second;
  bool is_valid;
  Position position;
  ENUVector velocity;
  GNSSQuality quality;
  float rtk_age;
  float rtk_ratio;

  // Example:
  // $PSTI,030,044606.000,A,2447.0924110,N,12100.5227860,E,103.323,0.00,0.00,0.00,180915,R,1.2,4.2*02

  // note: term offsets are one larger than in the reference because
  // the subsentence number is at offset 1

  // Field  Name  Example  Description
  // 1  UTC time  044606.000  UTC time in hhmmss.sss format (000000.00 ~ 235959.999)
  ok &= parse_time(&time.tm_hour, &time.tm_min, &second, buffer+term_offsets[2]);
  // 2  Status  A  Status
  // ‘V’ = Navigation receiver warning
  // ‘A’ = Data Valid
  ok &= parse_AV(&is_valid, buffer+term_offsets[3]);
  // 3  Latitude  2447.0924110  Latitude in dddmm.mmmmmmm format
  // Leading zeros transmitted
  ok &= parse_latlon(&position.latitude, buffer+term_offsets[4]);
  // 4  N/S indicator  N  Latitude hemisphere indicator
  // ‘N’ = North
  // ‘S’ = South
  ok &= parse_NS(&position.latitude, buffer+term_offsets[5]);
  // 5  Longitude  12100.5227860 Longitude in dddmm.mmmmmmm format
  // Leading zeros transmitted
  ok &= parse_latlon(&position.longitude, buffer+term_offsets[6]);
  // 6  E/W Indicator  E  Longitude hemisphere indicator
  // 'E' = East
  // 'W' = West
  ok &= parse_EW(&position.longitude, buffer+term_offsets[7]);
  // 7  Altitude  103.323  mean sea level (geoid), (‐9999.999 ~ 17999.999)
  ok &= parse_float(&position.altitude, buffer+term_offsets[8]);
  // 8  East Velocity  0.00  ‘East’ component of ENU velocity (m/s)
  ok &= parse_float(&velocity.east, buffer+term_offsets[9]);
  // 9  North Velocity  0.00  ‘North’ component of ENU velocity (m/s)
  ok &= parse_float(&velocity.north, buffer+term_offsets[10]);
  // 10  Up Velocity  0.00  ‘Up’ component of ENU velocity (m/s)
  ok &= parse_float(&velocity.up, buffer+term_offsets[11]);
  // 11  UTC Date  180915  UTC date of position fix, ddmmyy format
  ok &= parse_date(&time.tm_year, &time.tm_mon, &time.tm_mday, buffer+term_offsets[12]);
  // 12  Mode indicator  R  Mode indicator
  // ‘N’ = Data not valid
  // ‘A’ = Autonomous mode
  // ‘D’ = Differential mode
  // ‘E’ = Estimated (dead reckoning) mode
  // ‘M’ = Manual input mode
  // ‘S’ = Simulator mode
  // ‘F’ = Float RTK. Satellite system used in RTK mode, floating
  // integers
  // ‘R’ = Real Time Kinematic. System used in RTK mode with fixed
  // integers
  ok &= parse_PSTI030_mode(&quality, buffer+term_offsets[13]);
  // 13  RTK Age  1.2  Age of differential
  ok &= parse_float(&rtk_age, buffer+term_offsets[14]);
  // 14  RTK Ratio  4.2  AR ratio factor for validation
  ok &= parse_float(&rtk_ratio, buffer+term_offsets[15]);

  report_success(ok, sentence());
  if (!ok) {
    return;
  }

  time.tm_sec = (int)second;
  time.tm_isdst = 0;

  // notify relevant observers

  nmea_data->gnss_quality.set(gnssQualityStrings[quality]);
  nmea_data->rtk_age.set(rtk_age);
  nmea_data->rtk_ratio.set(rtk_ratio);

  if (is_valid) {
    nmea_data->position.set(position);
    nmea_data->datetime.set(mktime(&time));
    nmea_data->enu_velocity.set(velocity);
  }
}

void PSTI032SentenceParser::parse(char* buffer, int term_offsets[], int num_terms) {
  bool ok = true;

  struct tm time;
  float second;
  bool is_valid = false;
  ENUVector projection;
  GNSSQuality quality;
  float baseline_length;
  float baseline_course;

  char reconstruction[INPUT_BUFFER_LENGTH];
  reconstruct_nmea_sentence(reconstruction, buffer, term_offsets, num_terms);
  debugD("%s", reconstruction);

  // Example:
  // $PSTI,032,041457.000,170316,A,R,0.603,‐0.837,‐0.089,1.036,144.22,,,,,*30

  // note: term offsets are one larger than in the reference because
  // the subsentence number is at offset 1

  // Field  Name  Example  Description
  // 1  UTC time  041457.000  UTC time in hhmmss.sss format (000000.000~235959.999)
  ok &= parse_time(&time.tm_hour, &time.tm_min, &second, buffer+term_offsets[2]);
  // 2  UTC Date  170316  UTC date of position fix, ddmmyy format
  ok &= parse_date(&time.tm_year, &time.tm_mon, &time.tm_mday, buffer+term_offsets[3]);
  // 3  Status  A
  // Status
  // ‘V’ = Void
  // ‘A’ = Active
  ok &= parse_AV(&is_valid, buffer+term_offsets[4]);
  if (is_valid) {
    // 4  Mode indicator  R
    // Mode indicator
    // ‘F’ = Float RTK. System used in RTK mode with float ambiguity
    // ‘R’ = Real Time Kinematic. System used in RTK mode with fixed
    // ambiguity
    ok &= parse_PSTI030_mode(&quality, buffer+term_offsets[5]);
    // 5  East‐projection of
    // baseline  0.603  East‐projection of baseline, meters
    ok &= parse_float(&projection.east, buffer+term_offsets[6]);
    // 6  North‐projection of
    // baseline  ‐0.837  North‐projection of baseline, meters
    ok &= parse_float(&projection.north, buffer+term_offsets[7]);
    // 7  Up‐projection of
    // baseline  ‐0.089  Up‐projection of baseline, meters
    ok &= parse_float(&projection.up, buffer+term_offsets[8]);
    // 8  Baseline length  1.036  Baseline length, meters
    ok &= parse_float(&baseline_length, buffer+term_offsets[9]);
    // 9  Baseline course  144.22
    // Baseline course (angle between baseline vector and north
    // direction), degrees
    ok &= parse_float(&baseline_course, buffer+term_offsets[10]);
    // 10  Reserve    Reserve
    // 11  Reserve    Reserve
    // 12  Reserve    Reserve
    // 13  Reserve    Reserve
    // 14  Reserve    Reserve
  }

  report_success(ok, sentence());
  if (!ok) {
    return;
  }

  time.tm_sec = (int)second;
  time.tm_isdst = 0;

  if (is_valid) {
    nmea_data->datetime.set(mktime(&time));
    nmea_data->baseline_projection.set(projection);
    nmea_data->baseline_length.set(baseline_length);
    nmea_data->baseline_course.set(2*PI*baseline_course/360.);
    nmea_data->gnss_quality.set(gnssQualityStrings[quality]);
  }
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

void NMEAParser::handle(char c) {
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
        sentence_parsers[buffer]->parse(buffer, term_offsets, cur_term+1,
                                        sentence_parsers);
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
  return this->parity==checksum;
}
