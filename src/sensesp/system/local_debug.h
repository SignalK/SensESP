#ifndef _local_debug_H_
#define _local_debug_H_

#include "Arduino.h"
#include "Print.h"

namespace sensesp {

#ifndef DEBUG_DISABLED

#define rdebugA(fmt, ...)        \
  if (Debug.isActive(Debug.ANY)) \
  Serial.printf("(%s)(C%d) " fmt, __func__, xPortGetCoreID(), ##__VA_ARGS__)
#define rdebugP(fmt, ...)             \
  if (Debug.isActive(Debug.PROFILER)) \
  Serial.printf("(%s)(C%d) " fmt, __func__, xPortGetCoreID(), ##__VA_ARGS__)
#define rdebugV(fmt, ...)            \
  if (Debug.isActive(Debug.VERBOSE)) \
  Serial.printf("(%s)(C%d) " fmt, __func__, xPortGetCoreID(), ##__VA_ARGS__)
#define rdebugD(fmt, ...)          \
  if (Debug.isActive(Debug.DEBUG)) \
  Serial.printf("(%s)(C%d) " fmt, __func__, xPortGetCoreID(), ##__VA_ARGS__)
#define rdebugI(fmt, ...)         \
  if (Debug.isActive(Debug.INFO)) \
  Serial.printf("(%s)(C%d) " fmt, __func__, xPortGetCoreID(), ##__VA_ARGS__)
#define rdebugW(fmt, ...)            \
  if (Debug.isActive(Debug.WARNING)) \
  Serial.printf("(%s)(C%d) " fmt, __func__, xPortGetCoreID(), ##__VA_ARGS__)
#define rdebugE(fmt, ...)          \
  if (Debug.isActive(Debug.ERROR)) \
  Serial.printf("(%s)(C%d) " fmt, __func__, xPortGetCoreID(), ##__VA_ARGS__)

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

#define debugV(fmt, ...) rdebugVln(fmt, ##__VA_ARGS__)
#define debugD(fmt, ...) rdebugDln(fmt, ##__VA_ARGS__)
#define debugI(fmt, ...) rdebugIln(fmt, ##__VA_ARGS__)
#define debugW(fmt, ...) rdebugWln(fmt, ##__VA_ARGS__)
#define debugE(fmt, ...) rdebugEln(fmt, ##__VA_ARGS__)
#define debugA(fmt, ...) rdebugVln(fmt, ##__VA_ARGS__)

class LocalDebug {
 public:
  bool begin(String hostname, uint8_t startingDebugLevel = DEBUG);

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

#else  // DEBUG_DISABLED

#define debugA(...)
#define debugP(...)
#define debugV(...)
#define debugD(...)
#define debugI(...)
#define debugW(...)
#define debugE(...)

#endif

}  // namespace sensesp

#endif
