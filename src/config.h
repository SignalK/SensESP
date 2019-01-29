#ifndef _config_H_
#define _config_H_

// Wifi config portal timeout (seconds). The smaller the value, the faster
// the device will attempt to reconnect. If set too small, it might
// become impossible to actually configure the Wifi settings in the captive
// portal.
#define WIFI_CONFIG_PORTAL_TIMEOUT 180

// HTTP port for the configuration interface
#define HTTP_SERVER_PORT 80

// Password for Over-the-air (OTA) updates
#define OTA_PASSWORD "bonvoyage"

// LED pin

//#define LED_PIN LED_BUILTIN  // If your board doesn't have a defined LED_BUILTIN, comment this line out...
#define LED_PIN 2        // ... and uncomment this line, and change 13 to any LED pin you have. If you have none, leave it at 13.
#define LED_ACTIVE_LOW true


// if you want to use the optional my_config.h file
// to override configuration constants,
// uncomment the following line.

// #include "my_config.h"


#endif