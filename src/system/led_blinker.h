#ifndef _led_blinker_H_
#define _led_blinker_H_

#include <ReactESP.h>

class LedBlinker {
  private:
    int current_state = 0;
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
    LedBlinker();
};

#endif
