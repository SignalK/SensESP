#ifndef _SYSTEM_STATUS_CONSUMER_H_
#define _SYSTEM_STATUS_CONSUMER_H_

#include "net/networking.h"
#include "net/ws_client.h"
#include "system/valueconsumer.h"

/**
 * An interface for a subsystem providing feedback on the system state.
 * May be subclassed to implement e.g. a blinking LED or a display.
 */
class SystemStatusConsumer : public ValueConsumer<WifiState>,
                             public ValueConsumer<WSConnectionState>,
                             public ValueConsumer<int> {
 public:
  // ValueConsumer interface for ValueConsumer<WifiState> (Networking object
  // state updates)
  virtual void set_input(WifiState new_value, uint8_t input_channel = 0) = 0;
  // ValueConsumer interface for ValueConsumer<WSConnectionState>
  // (WSClient object state updates)
  virtual void set_input(WSConnectionState new_value,
                         uint8_t input_channel = 0) = 0;
  // ValueConsumer interface for ValueConsumer<int> (delta count producer
  // updates)
  virtual void set_input(int new_value, uint8_t input_channel = 0) = 0;
};

#endif
