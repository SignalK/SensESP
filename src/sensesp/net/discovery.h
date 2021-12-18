#ifndef _discovery_H_
#define _discovery_H_

#include "sensesp/system/startable.h"

namespace sensesp {

class MDNSDiscovery : public Startable {
 public:
  MDNSDiscovery() : Startable(0) {}
  virtual void start() override;
};

}  // namespace sensesp

#endif
