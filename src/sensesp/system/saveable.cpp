#include "saveable.h"

#include "SPIFFS.h"
#include "sensesp/system/hash.h"

namespace sensesp {

bool FileSystemSaveable::load() {
  if (config_path_ == "") {
    ESP_LOGI(__FILENAME__,
             "Not loading configuration: no config_path specified: %s",
             config_path_.c_str());
    return false;
  }
  String hash_path = String("/") + Base64Sha1(config_path_);

  String filename;
  if (!find_config_file(config_path_, filename)) {
    return false;
  }

  File f = SPIFFS.open(filename, "r");
  JsonDocument json_doc;
  auto error = deserializeJson(json_doc, f);
  f.close();
  if (error) {
    ESP_LOGW(__FILENAME__, "Could not parse configuration for %s",
             config_path_.c_str());
    return false;
  }  //
  JsonObject obj = json_doc.as<JsonObject>();
  if (!from_json(obj)) {
    ESP_LOGW(__FILENAME__, "Could not convert configuration to Json for %s",
             config_path_.c_str());
    return false;
  }
  ESP_LOGD(__FILENAME__, "Configuration loaded for %s", config_path_.c_str());
  return true;
}

bool FileSystemSaveable::save() {
  if (config_path_ == "") {
    return false;
  }
  String hash_path = String("/") + Base64Sha1(config_path_);

  // Delete any existing configuration files
  String filename;
  if (find_config_file(config_path_, filename)) {
    SPIFFS.remove(filename);
  }

  JsonDocument json_doc;
  JsonObject obj = json_doc.as<JsonObject>();
  if (!to_json(obj)) {
    ESP_LOGW(__FILENAME__, "Could not get configuration from json for %s",
             config_path_.c_str());
    return false;
  }
  File f = SPIFFS.open(hash_path, "w");
  serializeJson(obj, f);
  f.close();

  String str;
  serializeJson(json_doc, str);
  ESP_LOGI(__FILENAME__, "Configuration saved for %s: %s", config_path_.c_str(),
           str.c_str());

  return true;
}

bool FileSystemSaveable::remove() {
  if (config_path_ == "") {
    return false;
  }

  String filename;
  if (!find_config_file(config_path_, filename)) {
    return true;
  }

  String hash_path = String("/") + Base64Sha1(config_path_);

  if (SPIFFS.exists(hash_path)) {
    SPIFFS.remove(hash_path);
  }
  return true;
}

bool FileSystemSaveable::find_config_file(const String& config_path,
                                          String& filename) {
  String hash_path = String("/") + Base64Sha1(config_path);
  String paths_to_check[] = {hash_path, hash_path + "\n"};

  // Check for SensESP v2.4.0 and earlier versions
  for (const auto& path : paths_to_check) {
    if (SPIFFS.exists(path)) {
      filename = path;
      return true;
    }
  }

  // Check for SensESP v2.1.0 and earlier versions
  if (config_path.length() < 32 && SPIFFS.exists(config_path)) {
    filename = config_path;
    return true;
  }

  return false;
}

}  // namespace sensesp
