#ifndef _app_H_
#define _app_H_

#include <list>

#include "config.h"
#include "components/component.h"
#include "devices/device.h"
#include "net/http.h"
#include "net/ws_client.h"
#include "sensesp.h"
#include "system/led_blinker.h"
#include "system/signal_k.h"

class SensESPApp {
    std::list<Device*> devices;
    std::list<Component*> components;

    HTTPServer* http_server;
    LedBlinker led_blinker;
    SKDelta* sk_delta;
    WSClient* ws_client;

  public:
    SensESPApp();
    void enable();
};

extern SensESPApp* sensesp_app;

#endif
