#ifndef _nmea_parser_H_
#define _nmea_parser_H_

#include "Arduino.h"

#include <map>

#include "observablevalue.h"

constexpr int INPUT_BUFFER_LENGTH = 250;
constexpr int MAX_TERMS = 25;

struct Position {
  double latitude;
  double longitude;
  float altitude;
};

struct ENUVector {
  float east;
  float north;
  float up;
};

struct NMEAData {
  ObservableValue<Position> position;
  ObservableValue<String> gnss_quality;
  ObservableValue<int> num_satellites;
  ObservableValue<float> horizontal_dilution;
  ObservableValue<float> geoidal_separation;
  ObservableValue<float> dgps_age;
  ObservableValue<float> dgps_id;
  ObservableValue<time_t> datetime;
  ObservableValue<float> speed;
  ObservableValue<float> true_course;
  ObservableValue<float> variation;
  ObservableValue<ENUVector> enu_velocity;
  ObservableValue<float> rtk_age;
  ObservableValue<float> rtk_ratio;
  ObservableValue<ENUVector> baseline_projection;
  ObservableValue<float> baseline_length;
  ObservableValue<float> baseline_course;
};

class SentenceParser {
 public:
  SentenceParser(NMEAData* nmea_data);
  virtual void parse(char* buffer, int term_offsets[], int num_terms) = 0;
  virtual void parse(
    char* buffer, int term_offsets[], int num_terms,
    std::map<String, SentenceParser*>& sentence_parsers
  ) {
    parse(buffer, term_offsets, num_terms);
  }
  virtual const char* sentence() = 0;
 protected:
  NMEAData* nmea_data;
 private:
};

class GPGGASentenceParser : public SentenceParser {
 public:
  GPGGASentenceParser(NMEAData* nmea_data) : SentenceParser{nmea_data} {}
  void parse(char* buffer, int term_offsets[], int num_terms) override final;
  const char* sentence() { return "GPGGA"; }
 private:
};

class GPGLLSentenceParser : public SentenceParser {
 public:
  GPGLLSentenceParser(NMEAData* nmea_data) : SentenceParser{nmea_data} {}
  void parse(char* buffer, int term_offsets[], int num_terms) override final;
  const char* sentence() { return "GPGLL"; }
 private:
};

class GPRMCSentenceParser : public SentenceParser {
 public:
  GPRMCSentenceParser(NMEAData* nmea_data) : SentenceParser{nmea_data} {}
  void parse(char* buffer, int term_offsets[], int num_terms) override final;
  const char* sentence() { return "GPRMC"; }
 private:
};

//class GPVTGSentenceParser : public SentenceParser {
// public:
//  GPVTGSentenceParser(NMEAData* nmea_data) : SentenceParser{nmea_data} {}
//  void parse(char* buffer, int term_offsets[], int num_terms) override final;
//  const char* sentence() { return "GPVTG"; }
// private:
//};

class PSTISentenceParser : public SentenceParser {
 public:
  PSTISentenceParser(NMEAData* nmea_data) : SentenceParser{nmea_data} {}
  void parse(char* buffer, int term_offsets[], int num_terms) override final {}
  void parse(
    char* buffer, int term_offsets[], int num_terms,
    std::map<String, SentenceParser*>& sentence_parsers
  );
  const char* sentence() { return "PSTI"; }
 private:
};

class PSTI030SentenceParser : public SentenceParser {
 public:
  PSTI030SentenceParser(NMEAData* nmea_data) : SentenceParser{nmea_data} {}
  void parse(char* buffer, int term_offsets[], int num_terms) override final;
  const char* sentence() { return "PSTI,030"; }
 private:
};

class PSTI032SentenceParser : public SentenceParser {
 public:
  PSTI032SentenceParser(NMEAData* nmea_data) : SentenceParser{nmea_data} {}
  void parse(char* buffer, int term_offsets[], int num_terms) override final;
  const char* sentence() { return "PSTI,032"; }
 private:
};

class NMEAParser {
public:
  NMEAParser();
  void handle(char c);
  void add_sentence_parser(SentenceParser* parser);
private:
  void (NMEAParser::* current_state) (char);
  void state_start(char c);
  void state_in_term(char c);
  void state_in_checksum(char c);
  // current sentence
  char buffer[INPUT_BUFFER_LENGTH];
  // offset for each sentence term in the buffer
  int term_offsets[MAX_TERMS];
  // pointer for the next character in buffer
  int cur_offset;
  // pointer for the current term in buffer
  int cur_term;
  int parity;
  bool validate_checksum();
  std::map<String, SentenceParser*> sentence_parsers;
};


#endif
