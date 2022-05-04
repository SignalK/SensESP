#ifndef _configurable_H_
#define _configurable_H_

#include <ArduinoJson.h>

#include <map>

#include "Arduino.h"

namespace sensesp {

/**
 * @brief An object that is capable of having configuration data
 * that can be set remotely using a RESTful API, and can be persisted to the
 * local file system.
 */
class Configurable {
 public:
  /**
   * Create a configurable object who's configuration is saved using the
   * specified config_path.
   *
   * @param config_path The partial path used for the URL when configuring this
   * object remotely with a browser. The config_path also represents the path to
   * the local file where the configuration data is persisted. Given this, be
   * sure to chose a path that is both valid for URLs as well as file names. A
   * configuration path should always start with a forward slash.  Leaving the
   * config_path blank indicates that this object does not need or want
   * configuration data support.
   * @param description A description of the Configurable that can be used e.g.
   * in the web UI.
   */
  Configurable(String config_path = "", String description = "",
               int sort_order = 1000);

  const String config_path_;

  /**
   * Returns the current configuration data as a JsonObject. In
   * general, the current state of local member variables are
   * saved to a new object created with JsonDocument::as<JsonObject>()
   * and returned.
   */
  virtual void get_configuration(JsonObject& configObject);

  /**
   * Sets the current state of local member variables using
   * the data stored in config.
   */
  virtual bool set_configuration(const JsonObject& config);

  /**
   * Returns a configuration schema that specifies the key/value pairs
   * that can be expected when calling get_configuration(), or are
   * expected by set_configuration(). The schema will be in
   * JSON Schema format
   * @see https://json-schema.org
   */
  virtual String get_config_schema();

  /**
   * Returns a configuration description that is visible in WebUI
   * you can use HTML tags to make description more readable
   */
  String get_description() { return description_; }

  /**
   * @brief Set the description of the Configurable.
   */
  void set_description(String description) { description_ = description; }

  int get_sort_order() { return sort_order_; }

  void set_sort_order(int sort_order) { sort_order_ = sort_order; }

  /**
   * Persists the configuration returned by get_configuration()
   * to the local file system.
   */
  virtual void save_configuration();

 protected:
  /**
   * Loads a configuration previously saved with save_configuration() and
   * passes the configuration to set_configuration().
   */
  virtual void load_configuration();

  String description_ = "";
  /// The sort order of Configurable on the web UI. Lower numbers have
  /// preference.
  int sort_order_ = 1000;
};

extern std::map<String, Configurable*> configurables;

}  // namespace sensesp

#endif
