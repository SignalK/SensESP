#ifndef _remote_debug_H_
#define _remote_debug_H_


#include "system/startable.h"

class RemoteDebugger : public Startable {
 public:
  RemoteDebugger() : Startable{0} {}
  virtual void start() override;
};

#endif
