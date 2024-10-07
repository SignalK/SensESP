#ifndef SENSESP_SYSTEM_FILESYSTEM_H
#define SENSESP_SYSTEM_FILESYSTEM_H

#include "resettable.h"

namespace sensesp {

class Filesystem : public Resettable {
 public:
  Filesystem();
  ~Filesystem();
  virtual void reset() override;
};

}  // namespace sensesp

#endif
