#ifndef _remote_debug_H_
#define _remote_debug_H_


#include "system/enableable.h"

class RemoteDebugger : public Enableable {
 public:
  RemoteDebugger() : Enableable{0} {}
  virtual void enable() override;
};

#endif
