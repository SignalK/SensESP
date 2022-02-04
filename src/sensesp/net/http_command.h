
#ifndef _http_command_H_
#define _http_command_H_

#include "../system/observable.h"
#include "Arduino.h"
namespace sensesp {

class HTTPCommand : public Observable {
 public:
  HTTPCommand(String title, String name, bool mustConfirm) {
    title_ = title;
    name_ = name;
    mustConfirm_ = mustConfirm;
  }

  const bool get_mustConfirm() { return mustConfirm_; }
  const String get_title() { return title_; }
  const String get_name() { return name_; }

 private:
  String title_;
  String name_;
  bool mustConfirm_;
};
}  // namespace sensesp
#endif