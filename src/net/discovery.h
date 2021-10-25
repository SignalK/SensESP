#ifndef _discovery_H_
#define _discovery_H_

#include "system/startable.h"

class MDNSDiscovery : public Startable {
 public:
  MDNSDiscovery() : Startable(0) {}
  virtual void start() override;
};

#endif
