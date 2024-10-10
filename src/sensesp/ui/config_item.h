#ifndef SENSESP_SRC_SENSESP_NET_WEB_CONFIG_ITEM_H_
#define SENSESP_SRC_SENSESP_NET_WEB_CONFIG_ITEM_H_

#include <map>
#include <memory>
#include <vector>

#include "Arduino.h"
#include "ArduinoJson.h"
#include "sensesp/system/saveable.h"

namespace sensesp {

/**
 * Convert the variable type to a string representation of the type using
 * template specializations.
 **/
// an unknown type may not be rendered in Json editor
template <class T>
const char* get_schema_type_string(const T dummy) {
  return "unknown";
}
template <>
const char* get_schema_type_string(const int dummy);

template <>
const char* get_schema_type_string(const float dummy);

template <>
const char* get_schema_type_string(const String& dummy);

template <>
const char* get_schema_type_string(const bool dummy);

/**
 * @brief Template function to provide a configuration schema for a
 * ConfigItemT<T>.
 *
 * @tparam T
 * @param obj
 * @return const char*
 */
template <typename T>
const String ConfigSchema(const T& obj) {
  return "null";
}

template <typename T>
const String ConfigSchema(const std::shared_ptr<T>& obj) {
  return ConfigSchema(*obj);
}

template <typename T>
bool ConfigRequiresRestart(const T& obj) {
  return false;
}

template <typename T>
bool ConfigRequiresRestart(const std::shared_ptr<T>& obj) {
  return ConfigRequiresRestart(*obj);
}

// Forward declarations

template <typename T>
class ConfigItemT;

template <typename T>
std::shared_ptr<ConfigItemT<T>> ConfigItem(std::shared_ptr<T>);


class ConfigItemBase
    : virtual public std::enable_shared_from_this<ConfigItemBase> {
 public:
  const String& get_title() const { return title_; }
  ConfigItemBase* set_title(const String& title) {
    title_ = title;
    return this;
  }

  /**
   * Returns a configuration description that is visible in the web UI.
   * You can use HTML tags to make description more readable.
   */
  const String& get_description() const { return description_; }

  /**
   * @brief Set the description of the ConfigItemT.
   */
  ConfigItemBase* set_description(const String& description) {
    description_ = description;
    return this;
  }

  /**
   * Returns a configuration schema that specifies the key/value pairs
   * that can be expected when calling get_configuration(), or are
   * expected by set_configuration(). The schema will be in
   * JSON Schema format
   * @see https://json-schema.org
   */
  virtual const String get_config_schema() const {
    if (config_schema_ != "") {
      return config_schema_;
    }
    return get_default_config_schema();
  }

  int get_sort_order() const { return sort_order_; }

  ConfigItemBase* set_sort_order(int sort_order) {
    sort_order_ = sort_order;
    return this;
  }

  /**
   * @brief Return true if the ConfigItemT requires restart after saving for
   * the changes to take effect.
   */
  bool requires_restart() const { return requires_restart_; }

  /**
   * @brief Set the requires_restart flag.
   *
   * @param requires_restart
   */
  ConfigItemBase* set_requires_restart(bool requires_restart) {
    requires_restart_ = requires_restart;
    return this;
  }

  /**
   * @brief Get a single ConfigItemT by key.
   *
   * Return nullptr if not found.
   */
  static std::shared_ptr<ConfigItemBase> get_config_item(const String key) {
    auto it = ConfigItemBase::config_items_.find(key);
    if (it != ConfigItemBase::config_items_.end()) {
      return it->second;
    }
    return nullptr;
  }

  /**
   * @brief Get all config items as a vector.
   *
   */
  static std::unique_ptr<std::vector<std::shared_ptr<ConfigItemBase>>>
  get_config_items() {
    std::unique_ptr<std::vector<std::shared_ptr<ConfigItemBase>>>
        sorted_config_items(new std::vector<std::shared_ptr<ConfigItemBase>>());

    for (auto& it : ConfigItemBase::config_items_) {
      sorted_config_items->push_back(it.second);
    }
    std::sort(sorted_config_items->begin(), sorted_config_items->end(),
              [](std::shared_ptr<ConfigItemBase> a,
                 std::shared_ptr<ConfigItemBase> b) {
                return a->get_sort_order() < b->get_sort_order();
              });

    return sorted_config_items;
  }

  virtual bool load() = 0;
  virtual bool refresh() = 0;
  virtual bool save() = 0;
  virtual bool to_json(JsonObject& config) const = 0;
  virtual bool from_json(const JsonObject& config) const = 0;
  virtual const String& get_config_path() const = 0;

 protected:
  static std::map<String, std::shared_ptr<ConfigItemBase>> config_items_;

  /// The path of the ConfigItemT. This is used to identify the ConfigItemT.
  String config_path_ = "";
  /// The configuration schema of the ConfigItemT in serialized JSON Schema
  /// format.
  String config_schema_ = "";
  /// Title of the ConfigItemT to be displayed on the web UI.
  String title_ = "";
  /// Description of the ConfigItemT to be displayed on the web UI.
  String description_ = "";
  /// The sort order of ConfigItemT. Lower numbers have precedence.
  int sort_order_ = 1000;
  /// Flag to indicate whether the ConfigItemT setting requires a restart
  /// to take effect after saving the configuration.
  bool requires_restart_ = false;

  virtual const String get_default_config_schema() const = 0;

  template <typename T>
  friend std::shared_ptr<ConfigItemT<T>> ConfigItem(std::shared_ptr<T>);
};

/**
 * @brief Class providing sufficient data for rendering a config card in the
 * frontend.
 *
 */
template <typename T>
class ConfigItemT : public ConfigItemBase {
  // Template classes must inherit from Serializable and Saveable.
  static_assert(std::is_base_of<Serializable, T>::value,
                "T must inherit from Serializable");
  static_assert(std::is_base_of<Saveable, T>::value,
                "T must inherit from Saveable");

 public:
  ConfigItemT(std::shared_ptr<T> config_object)
      : ConfigItemBase(), config_object_{config_object} {}

  virtual bool to_json(JsonObject& config) const override {
    return config_object_->to_json(config);
  }

  virtual bool from_json(const JsonObject& config) const override {
    return config_object_->from_json(config);
  }

  virtual bool load() override { return config_object_->load(); }
  virtual bool refresh() override { return config_object_->refresh(); }
  virtual bool save() override { return config_object_->save(); }

  virtual const String& get_config_path() const override {
    return config_object_->get_config_path();
  }

  ConfigItemT<T>* set_title(const String& title) {
    ConfigItemBase::set_title(title);
    return this;
  }

  ConfigItemT<T>* set_description(const String& description) {
    ConfigItemBase::set_description(description);
    return this;
  }

  ConfigItemT<T>* set_sort_order(int sort_order) {
    ConfigItemBase::set_sort_order(sort_order);
    return this;
  }

  ConfigItemT<T>* set_requires_restart(bool requires_restart) {
    ConfigItemBase::set_requires_restart(requires_restart);
    return this;
  }

  ConfigItemT<T>* set_config_schema(const String& config_schema) {
    config_schema_ = config_schema;
    return this;
  }

  /**
   * @brief Get the object that this ConfigItemT is managing.
   *
   * @return T*
   */
  T* get_config_object() { return config_object_; }

 protected:
  // The object that this ConfigItemT is managing.
  std::shared_ptr<T> config_object_;
  /**
   * @brief Get the default configuration schema.
   *
   * @return String
   */
  const String get_default_config_schema() const {
    String schema = ConfigSchema(*config_object_);
    return schema;
  }

};

/**
 * @brief Register a ConfigItemT with the ConfigItemBase.
 *
 * Note, this function only exists to avoid having to provide the template
 * argument when creating a ConfigItemT. It should be possible to remove it
 * once C++17 is supported.
 *
 * @tparam T
 * @param config_object
 * @param title
 * @param description
 * @param sort_order
 * @return T*
 */
template <typename T>
std::shared_ptr<ConfigItemT<T>> ConfigItem(std::shared_ptr<T> config_object) {
  ESP_LOGD(__FILENAME__, "Registering ConfigItemT with path %s",
           config_object->get_config_path().c_str());
  auto config_item = std::make_shared<ConfigItemT<T>>(config_object);
  auto base_sptr = std::static_pointer_cast<ConfigItemBase>(config_item);
  ConfigItemBase::config_items_[config_object->get_config_path()] = base_sptr;
  return config_item;
}

// Unsafe: We don't know whether other shared_ptrs to the same object exist!
template <typename T>
std::shared_ptr<ConfigItemT<T>> ConfigItem(T* config_object) {
  auto config_object_sptr = std::shared_ptr<T>(config_object);
  return ConfigItem(config_object_sptr);
}

}  // namespace sensesp

#endif  // SENSESP_SRC_SENSESP_NET_WEB_CONFIG_ITEM_H_
