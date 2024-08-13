#ifndef _configurable_H_
#define _configurable_H_

#include "sensesp.h"

#include <ArduinoJson.h>
#include <map>
#include <vector>

#include "Arduino.h"

enum class ConfigurableResult {
  kError,
  kOk,
  kPending,
};

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
   * @brief Return true if the Configurable is asynchronous.
   *
   * If the Configurable is asynchronous, async_get_configuration() and
   * async_set_configuration() must be used instead of the standard
   * get_configuration() and set_configuration() functions.
   *
   * @return true
   * @return false
   */
  virtual bool is_async() { return false; }

  /**
   * @brief Return true if the Configurable requires restart after saving.
   *
   */
  bool requires_restart() { return requires_restart_; }

  /**
   * @brief Set the requires_restart flag.
   *
   * @param requires_restart
   */
  void set_requires_restart(bool requires_restart) {
    requires_restart_ = requires_restart;
  }

  /**
   * Returns the current configuration data as a JsonObject. In
   * general, the current state of local member variables are
   * saved to a new object created with JsonDocument::as<JsonObject>()
   * and returned.
   */
  virtual void get_configuration(JsonObject& configObject) {
    ESP_LOGW(__FILENAME__, "WARNING: get_configuration not defined");
  }

  /**
   * @brief Initiate an asynchronous request to get the current configuration.
   *
   * Override to implement support for asynchronous configuration.
   *
   * @return ConfigurableResult
   */
  virtual ConfigurableResult async_get_configuration() {
    ESP_LOGE("Configurable", "async_get_configuration not implemented");
    return ConfigurableResult::kError;
  }

  /**
   * @brief Return the current status of the last async_get_configuration()
   * call.
   *
   * Override to implement support for polling.
   *
   */
  virtual ConfigurableResult poll_get_result(JsonObject& config) {
    ESP_LOGE("Configurable", "poll_get_result not implemented");
    return ConfigurableResult::kError;
  }

  /**
   * Sets the current state of local member variables using
   * the data stored in config.
   */
  virtual bool set_configuration(const JsonObject& config);

  /**
   * @brief Initiate an asynchronous request to set the configuration.
   *
   * Override to implement support for asynchronous configuration.
   *
   * @param config
   * @return ConfigurableResult
   */
  virtual ConfigurableResult async_set_configuration(const JsonObject& config) {
    ESP_LOGE("Configurable", "async_set_configuration not implemented");
    return ConfigurableResult::kError;
  }

  /**
   * @brief Return the current status of the last async set_configuration call.
   *
   * Override to implement support for polling.
   *
   */
  virtual ConfigurableResult poll_set_result() {
    ESP_LOGE("Configurable", "poll_set_result not implemented");
    return ConfigurableResult::kError;
  }

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
  Configurable* set_description(String description) {
    description_ = description;
    return this;
  }

  int get_sort_order() { return sort_order_; }

  Configurable* set_sort_order(int sort_order) {
    sort_order_ = sort_order;
    return this;
  }

  /**
   * Persists the configuration returned by get_configuration()
   * to the local file system.
   */
  virtual void save_configuration();

  /**
   * @brief Clear existing configuration data.
   *
   */
  virtual void clear_configuration();

  /**
   * @brief Get configurables.
   *
   */
  static std::vector<Configurable*> get_configurables() {
    // Sort configurables by sort_order
    std::vector<Configurable*> sorted_configurables;
    for (auto& it : Configurable::configurables_) {
      sorted_configurables.push_back(it.second);
    }
    std::sort(sorted_configurables.begin(), sorted_configurables.end(),
              [](Configurable* a, Configurable* b) {
                return a->get_sort_order() < b->get_sort_order();
              });

    return sorted_configurables;
  }

  /**
   * @brief Get a single Configurable by key.
   *
   * Return nullptr if not found.
   */
  static Configurable* get_configurable(String key) {
    auto it = Configurable::configurables_.find(key);
    if (it != Configurable::configurables_.end()) {
      return it->second;
    }
    return nullptr;
  }

 protected:
  static std::map<String, Configurable*> configurables_;
  /**
   * Loads a configuration previously saved with save_configuration() and
   * passes the configuration to set_configuration(). This function is typically
   * only called by a child class constructor.
   */
  virtual void load_configuration();

  String description_ = "";
  /// The sort order of Configurable on the web UI. Lower numbers have
  /// preference.
  int sort_order_ = 1000;
  /// @brief Flag to indicate if the Configurable requires a restart after
  /// saving the configuration.
  bool requires_restart_ = false;
};

}  // namespace sensesp

#endif
