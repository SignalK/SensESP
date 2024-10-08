#ifndef SENSESP_SYSTEM_SAVEABLE_H_
#define SENSESP_SYSTEM_SAVEABLE_H_

#include "sensesp.h"

#include <ArduinoJson.h>

#include "Arduino.h"
#include "serializable.h"

namespace sensesp {

/**
 * @brief Interface for saveable objects.
 *
 * Saveable objects can be saved and loaded from a persistent storage.
 */
class Saveable {
 public:
  Saveable(const String& config_path) : config_path_{config_path} {}

  /**
   * @brief Load and populate the object from a persistent storage.
   *
   * @return true Object loaded successfully
   * @return false
   */
  virtual bool load() { return false; }
  /**
   * @brief Refresh the object. This may or may not access the persistent
   * storage but is not expected to overwrite the object's state.
   *
   * This is useful when the object's state may have changed outside of the
   * object's control.
   *
   * @return true Object refreshed successfully
   * @return false
   */
  virtual bool refresh() { return false; }
  /**
   * @brief Save the object to a persistent storage.
   *
   * @return true
   * @return false
   */
  virtual bool save() { return false; }
  /**
   * @brief Remove the object from a persistent storage.
   *
   * @return true
   * @return false
   */
  virtual bool remove() { return false; }

  const String& get_config_path() const { return config_path_; }

 protected:
  const String config_path_;
};

class FileSystemSaveable : public Saveable, virtual public Serializable {
 public:
  FileSystemSaveable(const String& config_path) : Saveable(config_path) {}

  virtual bool load() override;
  virtual bool save() override;
  virtual bool remove() override;

  bool find_config_file(const String& config_path, String& filename);
};

}  // namespace sensesp

#endif  // SENSESP_SYSTEM_SAVEABLE_H_
