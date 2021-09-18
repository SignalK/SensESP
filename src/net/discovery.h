#ifndef _discovery_H_
#define _discovery_H_

#include "system/enableable.h"

class MDNSDiscovery : public Enableable {
 public:
  MDNSDiscovery() : Enableable(0) {}
  virtual void enable() override;
};

#endif
