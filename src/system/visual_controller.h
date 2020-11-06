#ifndef _VISUAL_CONTROLLER_H_
#define _VISUAL_CONTROLLER_H_

#include "net/networking.h"
#include "net/ws_client.h"
#include "valueconsumer.h"

class VisualController : public ValueConsumer<WifiState>,
                         public ValueConsumer<WSConnectionState>,
                         public ValueConsumer<int> {
 public:
  // ValueConsumer interface for ValueConsumer<WifiState> (Networking object
  // state updates)
  virtual void set_input(WifiState new_value, uint8_t input_channel = 0) override;
  // ValueConsumer interface for ValueConsumer<WSConnectionState>
  // (WSClient object state updates)
  virtual void set_input(WSConnectionState new_value,
                 uint8_t input_channel = 0) override;
  // ValueConsumer interface for ValueConsumer<int> (delta count producer
  // updates)
  virtual void set_input(int new_value, uint8_t input_channel = 0) override;
};

#endif
