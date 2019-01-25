#ifndef _app_H_
#define _app_H_

#include "config.h"
#include "sensesp.h"
#include "components/component.h"
#include "devices/device.h"
#include "net/ws_client.h"
#include "system/led_blinker.h"
#include "system/signal_k.h"

#include <list>


class SensESPApp {
    std::list<Device*> devices;
    std::list<Component*> components;

    LedBlinker led_blinker;
    WSClient* ws_client;
    SKDelta* sk_delta;

  public:
    SensESPApp();
    void enable();
};

extern SensESPApp* sensesp_app;

#endif
