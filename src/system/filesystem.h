#ifndef _filesystem_H_
#define _filesystem_H_

#include "system/resettable.h"

class Filesystem : public Resettable {
 public:
  Filesystem();
  virtual void reset() override;
};

#endif
