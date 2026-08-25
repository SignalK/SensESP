#ifndef SENSESP_UI_UI_BUTTON_H
#define SENSESP_UI_UI_BUTTON_H

#include <map>
#include <memory>

#include "Arduino.h"
#include "sensesp/system/observable.h"

namespace sensesp {

/**
 * @brief UIButton implements a button interface on the web UI.
 *
 * Each UIButton object creates a button in the "Control" tab of the web UI.
 * When the button is clicked, the object's observers are notified.
 */
class UIButton : public Observable {
 public:
  UIButton(String title, String name, bool must_confirm)
      : title_(title), name_(name), must_confirm_(must_confirm) {}

  const bool get_must_confirm() { return must_confirm_; }
  const String get_title() { return title_; }
  const String get_name() { return name_; }

  static const std::map<String, std::shared_ptr<UIButton>>& get_ui_buttons() {
    return ui_buttons_;
  }

  /**
   * @brief Create a button and register it for the web UI.
   *
   * Call during setup, before the HTTP server starts serving: the registry
   * is read by the web UI handlers without locking. A duplicate name
   * replaces the previously registered button.
   *
   * Names up to 64 characters can be triggered through the web UI; a
   * longer name is listed but its clicks are rejected.
   */
  static UIButton* add(String name, String title, bool must_confirm = true) {
    if (ui_buttons_.count(name) != 0) {
      ESP_LOGW("UIButton",
               "Duplicate button name '%s' replaces the earlier button",
               name.c_str());
    }
    auto new_cmd = std::make_shared<UIButton>(title, name, must_confirm);
    ui_buttons_[name] = new_cmd;

    return new_cmd.get();
  }

  /**
   * @brief Empty the button registry.
   *
   * Releases the registry's shared_ptr references. Intended for clean app
   * restart and test isolation; not for normal runtime use.
   *
   * Like ConfigItemBase and unlike the raw-pointer registries, this registry
   * owns its entries via shared_ptr, so there is no unregister-on-destruction:
   * clearing the map releases the objects.
   */
  static void clear_registry();

 protected:
  String title_;
  String name_;
  bool must_confirm_;

  static std::map<String, std::shared_ptr<UIButton>> ui_buttons_;
};

}  // namespace sensesp

#endif
