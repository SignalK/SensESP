#ifndef _SENSESP_UI_UI_BUTTON_H_
#define _SENSESP_UI_UI_BUTTON_H_

#include <map>

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

  static const std::map<String, UIButton*>& get_ui_buttons() {
    return ui_buttons_;
  }

  static UIButton* add(String name, String title, bool must_confirm = true) {
    UIButton* new_cmd = new UIButton(title, name, must_confirm);
    ui_buttons_[name] = new_cmd;

    return new_cmd;
  }

 protected:
  String title_;
  String name_;
  bool must_confirm_;

  static std::map<String, UIButton*> ui_buttons_;
};

}  // namespace sensesp

#endif
