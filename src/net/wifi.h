#ifndef _wifi_H_
#define _wifi_H_

#include "Arduino.h"
#include <WiFiManager.h>

#include "system/led_blinker.h"

void setup_wifi(LedBlinker led_blinker);

#endif