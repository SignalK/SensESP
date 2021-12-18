#ifndef _filesystem_H_
#define _filesystem_H_

#include "resettable.h"

namespace sensesp {

class Filesystem : public Resettable {
 public:
  Filesystem();
  virtual void reset() override;
};

}  // namespace sensesp

#endif
