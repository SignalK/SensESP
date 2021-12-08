#ifndef _ota_H_
#define _ota_H_

#include "system/startable.h"

namespace sensesp {

class OTA : public Startable {
 public:
  /**
   * @brief Construct a new OTA (Over-the-air update) object
   *
   * @param password A password to be used for the OTA update.
   */
  OTA(const char* password) : Startable{0}, password_{password} {}
  virtual void start() override;

 private:
  const char* password_;
  static void handle_ota();
};

}  // namespace sensesp

#endif
