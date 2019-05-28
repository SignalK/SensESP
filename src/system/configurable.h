#ifndef _configurable_H_
#define _configurable_H_

#include <map>

#include "Arduino.h"
#include <ArduinoJson.h>


/**
 * A Configurable is any object that is capable of having configuration data that
 * can be set remotely using a RESTful API, and can be persisted to the 
 * local file system.
 */
class Configurable {
 public:
  /**
   * Create a configurable object who's configuration is saved using the
   * specified config_path.
   * @param config_path The partial path used for the URL when configuring this object
   *    remotely with a browser. The config_path also represents the path to the local
   *    file where the configuration data is persisted. Given this, be sure to
   *    chose a path that is both valid for URLs as well as file names. A configuration
   *    path should always start with a forward slash.  Leaving the config_path blank
   *    indicates that this object does not need or want configuration data support.
   */
  Configurable(String config_path);
  
  const String config_path;


  /**
   * Returns the current configuration data as a JsonObject. In
   * general, the current state of local member variables are 
   * saved to a new object created with JsonBuffer::createObject() 
   * and returned.
   */  
  virtual JsonObject& get_configuration(JsonBuffer& buf);



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

 };

extern std::map<String, Configurable*> configurables;

#endif
