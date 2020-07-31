#ifndef _led_blinker_H_
#define _led_blinker_H_

#include <ReactESP.h>
#include "sensesp_app_options.h"

class LedBlinker {
  private:
    int current_state = 0;
    int pin = 0;
    bool enabled = true;
    LedIntervals_T intervals;
    RepeatReaction* blinker = nullptr;
    void remove_blinker();
  protected:
    void set_state(int new_state);
  public:
    void set_wifi_connected();
    void set_wifi_disconnected();
    void set_server_connected();
    inline void set_server_disconnected() {
      set_wifi_connected();
    }
    void flip();
    LedBlinker(int pin, bool enabled, LedIntervals_T intervals);
};

#endif
