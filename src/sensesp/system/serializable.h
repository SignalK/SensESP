#ifndef SENSESP_SYSTEM_SERIALIZABLE_H_
#define SENSESP_SYSTEM_SERIALIZABLE_H_

#include "sensesp.h"

#include <ArduinoJson.h>

#include "Arduino.h"


namespace sensesp {

/**
 * @brief Interface for serializable objects.
 *
 * Serializable objects can be converted to and from JSON, useful for
 * configuration and persistence.
 */
class Serializable {
 public:
  /**
   * Serializes the current object data into a JsonObject.
   */
  virtual bool to_json(JsonObject& root) { return false; }
  /**
   * Deserializes the current object data from a JsonObject.
   */
  virtual bool from_json(const JsonObject& root) { return false; }
};


} // namespace sensesp

#endif // SENSESP_SYSTEM_SERIALIZABLE_H_
