#include "sensesp.h"

#include "configurable.h"

#include "SPIFFS.h"
#include "sensesp/system/hash.h"

namespace sensesp {

std::map<String, Configurable*> Configurable::configurables_;

Configurable::Configurable(String config_path, String description,
                           int sort_order)
    : config_path_{config_path},
      description_{description},
      sort_order_{sort_order} {
  if (config_path != "") {
    Configurable* confable = Configurable::get_configurable(config_path);
    if (confable != nullptr) {
      ESP_LOGW(__FILENAME__, "WARNING: Overriding id %s", config_path.c_str());
    }
    Configurable::configurables_[config_path] = this;
  }
}

// Sets and saves the configuration
bool Configurable::set_configuration(const JsonObject& config) {
  ESP_LOGW(__FILENAME__, "WARNING: set_configuration not defined for this Class");
  return false;
}

String Configurable::get_config_schema() { return "{}"; }

void Configurable::load_configuration() {
  if (config_path_ == "") {
    ESP_LOGI(__FILENAME__,
             "Not loading configuration: no config_path specified: %s",
             config_path_.c_str());
    return;
  }
  String hash_path = String("/") + Base64Sha1(config_path_);

  const String* filename;

  if (SPIFFS.exists(hash_path)) {
    filename = &hash_path;
    ESP_LOGD(__FILENAME__, "Loading configuration for path '%s' from '%s'",
             config_path_.c_str(), hash_path.c_str());
  } else if (SPIFFS.exists(hash_path + "\n")) {
    // Up to SensESP v2.4.0, the config path hash had an accidental newline
    // appended to it.
    hash_path += "\n";
    filename = &hash_path;
    ESP_LOGD(__FILENAME__, "Loading configuration for path '%s' from '%s'",
             config_path_.c_str(), hash_path.c_str());
  } else if (config_path_.length() < 32 && SPIFFS.exists(config_path_)) {
    // Prior to SensESP v2.1.0, the config path was a plain filename.
    filename = &config_path_;
    ESP_LOGD(__FILENAME__, "Loading configuration for path %s",
             config_path_.c_str());
  } else {
    ESP_LOGI(__FILENAME__, "Could not find configuration for path %s",
             config_path_.c_str());
    return;
  }

  File f = SPIFFS.open(*filename, "r");
  JsonDocument jsonDoc;
  auto error = deserializeJson(jsonDoc, f);
  if (error) {
    ESP_LOGW(__FILENAME__, "WARNING: Could not parse configuration for %s",
             config_path_.c_str());
    return;
  }  //
  if (!set_configuration(jsonDoc.as<JsonObject>())) {
    ESP_LOGW(__FILENAME__, "WARNING: Could not set configuration for %s",
             config_path_.c_str());
  }
  f.close();
}

void Configurable::save_configuration() {
  if (config_path_ == "") {
    ESP_LOGI(__FILENAME__,
             "WARNING: Could not save configuration (config_path not set)");
  }
  String hash_path = String("/") + Base64Sha1(config_path_);

  if (config_path_.length() < 32 && SPIFFS.exists(config_path_)) {
    ESP_LOGD(__FILENAME__, "Deleting legacy configuration file %s",
             config_path_.c_str());
    SPIFFS.remove(config_path_);
  }

  ESP_LOGD(__FILENAME__, "Saving configuration path %s to file %s",
           config_path_.c_str(), hash_path.c_str());

  JsonDocument jsonDoc;
  JsonObject obj = jsonDoc["root"].to<JsonObject>();
  get_configuration(obj);
  File f = SPIFFS.open(hash_path, "w");
  serializeJson(obj, f);
  f.close();
}

void Configurable::clear_configuration() {
  if (config_path_ == "") {
    ESP_LOGI(__FILENAME__,
             "WARNING: Could not clear configuration (config_path not set)");
  }
  String hash_path = String("/") + Base64Sha1(config_path_);

  if (SPIFFS.exists(hash_path)) {
    ESP_LOGD(__FILENAME__, "Deleting configuration file %s", hash_path.c_str());
    SPIFFS.remove(hash_path);
  }
}

}  // namespace sensesp
