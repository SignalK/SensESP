#ifndef _smart_switch_controller_h
#define _smart_switch_controller_h

#include "sensesp/signalk/signalk_put_request.h"
#include "sensesp/system/lambda_consumer.h"
#include "sensesp/system/valueconsumer.h"
#include "sensesp/transforms/click_type.h"
#include "sensesp/transforms/truth_text.h"
#include "sensesp/ui/config_item.h"

namespace sensesp {

/**
 * @brief A high level transform designed to control a digital output
 * (such as a relay) via manual button presses or programatic commands.
 *
 * To accomplish this, the controller accepts inputs from a generic boolean
 * producer (usually a SignalK listener), as well as String and ClickType
 * inputs. The latter allows a physical button to control the load as well as
 * add special behaviors to the sensor application. In particular, a double
 * click can be configured to set the state of multiple signal k paths, and
 * an ultra long press of the button will cause the MCU to reboot.
 * <p>A SmartSwitchController object behaves differently depending on the type
 * of input it receives. If the input is a boolean or a "valid truth type"
 * (which is a specialized type of boolean - see below), SmartSwitchController's
 * output will be "on" if the input is "true", or "off" if the input is "false".
 * If the input is a ClickType, or a String that's NOT a "valid truth type",
 * SmartSwitchController's output will simply toggle the output back and forth
 * between "on" and "off" with each input. There are two exceptions to this if
 * the input is a ClickType:
 * <ol>
 *   <li>A value of ClickTypes::DoubleClick will toggle the output as well as
 *       send PUT requests to synchronize zero or more paths that are configured
 * to do so</li> <li>A value of ClickTypes::UltraLongSingleClick will reboot the
 * MCU.</li>
 * </ol>
 * <p>Incoming String values are evaluated to see if they represent a "valid
 * truth type". Examples include "on", "ON", "off", "true", "false", "one", "1",
 * "123" (or any other string that represents a non-zero value), etc. Case is
 * insensitive. Any incoming String that doesn't evaluate to a "valid truth
 * type" will be treated as a "single click", and will toggle the output btween
 * "on" and "off".
 * @see TextToTruth
 * @see ClickType
 */
class SmartSwitchController : public ValueProducer<bool>, FileSystemSaveable {
 public:
  /**
   * The constructor
   * @param auto_initialize If TRUE, the controller will emit an
   *  initial "off" status when enabled. This is generally the
   *  desired case unless this controller is mirroring the state
   *  of a remote load.
   * @param config_path The path to save configuration data (blank for
   *   no saving)
   * @param sk_sync_paths An optional array of Signal K paths that should
   *   synchronize their status whenever a double click ClickType is received.
   *   Each path listed will have a PUT request issued to set the status to
   *   be the same as this SmartSwitchController each time a double click
   * occurs. This list, if specified, should have a zero length string as its
   * last entry.
   */
  SmartSwitchController(bool auto_initialize = true, String config_path = "",
                        const char* sk_sync_paths[] = NULL);

  // For reading and writing the configuration of this transformation
  virtual bool to_json(JsonObject& doc) override;
  virtual bool from_json(const JsonObject& config) override;

 public:
  LambdaConsumer<ClickTypes> click_consumer_{[this](ClickTypes new_value) {
    if (!ClickType::is_click(new_value)) {
      // Ignore button presses (we only want interpreted clicks)
      return;
    }

    if (new_value == ClickTypes::UltraLongSingleClick) {
      // Long clicks reboot the system...
      ESP.restart();
      return;
    }

    // All other click types toggle the current state...
    this->is_on_ = !this->is_on_;
    this->emit(this->is_on_);

    if (new_value == ClickTypes::DoubleClick) {
      // Sync any specified sync paths...
      for (auto& path : sync_paths_) {
        ESP_LOGD(__FILENAME__, "Sync status to %s", path.sk_sync_path_.c_str());
        path.put_request_->set(this->is_on_);
      }
    }
  }};

  LambdaConsumer<String> truthy_string_consumer_{[this](String new_value) {
    if (TextToTruth::is_valid_true(new_value)) {
      this->is_on_ = true;
    } else if (TextToTruth::is_valid_false(new_value)) {
      this->is_on_ = false;
    } else {
      // All other values simply toggle...
      this->is_on_ = !this->is_on_;
    }
    this->emit(this->is_on_);
  }};

  LambdaConsumer<bool> swich_consumer_{[this](bool value) {
    this->is_on_ = value;
    this->emit(is_on_);
  }};

  /// Used to store configuration internally.
  class SyncPath {
   public:
    String sk_sync_path_;
    std::shared_ptr<BoolSKPutRequest> put_request_;

    SyncPath();
    SyncPath(String sk_sync_path);

    friend bool operator<(const SyncPath& lhs, const SyncPath& rhs) {
      return lhs.sk_sync_path_ < rhs.sk_sync_path_;
    }
  };

 protected:
  bool is_on_ = false;
  bool auto_initialize_;
  std::set<SyncPath> sync_paths_;
};

const String ConfigSchema(const SmartSwitchController& obj) {
  return R"({"type":"object","properties":{"sync_paths":{"title":"Sync on double click","type":"array","items":{"type":"string"}}}  })";
}

}  // namespace sensesp

#endif
