#ifndef _local_debug_H_
#define _local_debug_H_

#include "esp32-hal-log.h"

#include "Arduino.h"
#include "Print.h"

#define rdebugA(fmt, ...) ESP_LOGV(__FILENAME__, fmt, ##__VA_ARGS__)
#define rdebugP(fmt, ...) ESP_LOGV(__FILENAME__, fmt, ##__VA_ARGS__)
#define rdebugV(fmt, ...) ESP_LOGV(__FILENAME__, fmt, ##__VA_ARGS__)
#define rdebugD(fmt, ...) ESP_LOGD(__FILENAME__, fmt, ##__VA_ARGS__)
#define rdebugI(fmt, ...) ESP_LOGI(__FILENAME__, fmt, ##__VA_ARGS__)
#define rdebugW(fmt, ...) ESP_LOGW(__FILENAME__, fmt, ##__VA_ARGS__)
#define rdebugE(fmt, ...) ESP_LOGE(__FILENAME__, fmt, ##__VA_ARGS__)

// With newline

#define rdebugAln(fmt, ...) rdebugA(fmt "\n", ##__VA_ARGS__)
#define rdebugPln(fmt, ...) rdebugP(fmt "\n", ##__VA_ARGS__)
#define rdebugVln(fmt, ...) rdebugV(fmt "\n", ##__VA_ARGS__)
#define rdebugDln(fmt, ...) rdebugD(fmt "\n", ##__VA_ARGS__)
#define rdebugIln(fmt, ...) rdebugI(fmt "\n", ##__VA_ARGS__)
#define rdebugWln(fmt, ...) rdebugW(fmt "\n", ##__VA_ARGS__)
#define rdebugEln(fmt, ...) rdebugE(fmt "\n", ##__VA_ARGS__)

// New way: To compatibility with SerialDebug (can use RemoteDebug or
// SerialDebug) This is my favorite :)

#define debugV(fmt, ...) rdebugV(fmt, ##__VA_ARGS__)
#define debugD(fmt, ...) rdebugD(fmt, ##__VA_ARGS__)
#define debugI(fmt, ...) rdebugI(fmt, ##__VA_ARGS__)
#define debugW(fmt, ...) rdebugW(fmt, ##__VA_ARGS__)
#define debugE(fmt, ...) rdebugE(fmt, ##__VA_ARGS__)
#define debugA(fmt, ...) rdebugV(fmt, ##__VA_ARGS__)

namespace sensesp {

class LocalDebug {
 public:
  bool begin(uint8_t startingDebugLevel = DEBUG);

  void setSerialEnabled(bool enable) {}  // No-op

  void setResetCmdEnabled(bool enable) {}  // No-op

  boolean isActive(uint8_t debugLevel = DEBUG);

  // Debug levels

  static const uint8_t PROFILER =
      0;  // Used for show time of execution of pieces of code(profiler)
  static const uint8_t VERBOSE = 1;  // Used for show verboses messages
  static const uint8_t DEBUG = 2;    // Used for show debug messages
  static const uint8_t INFO = 3;     // Used for show info messages
  static const uint8_t WARNING = 4;  // Used for show warning messages
  static const uint8_t ERROR = 5;    // Used for show error messages
  static const uint8_t ANY =
      6;  // Used for show always messages, for any current debug level

 private:
  uint8_t lastDebugLevel_ = DEBUG;
};

}  // namespace sensesp

#endif
