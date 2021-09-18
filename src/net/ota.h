#ifndef _ota_H_
#define _ota_H_

#include "system/enableable.h"

class OTA : public Enableable {
 public:
  OTA() : Enableable{0} {}
  virtual void enable() override;
 private:
  static void handle_ota();
};

#endif
