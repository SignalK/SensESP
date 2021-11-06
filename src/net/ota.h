#ifndef _ota_H_
#define _ota_H_

#include "system/startable.h"

namespace sensesp {

class OTA : public Startable {
 public:
  OTA() : Startable{0} {}
  virtual void start() override;

 private:
  static void handle_ota();
};

}  // namespace sensesp

#endif
