---
layout: default
title: Features
nav_order: 40
---

# Features

## Button Handling

Commonly used ESP32 devices such as the [SH-ESP32](https://docs.hatlabs.fi/sh-esp32/) or the DevKitC variants have a press button on the GPIO pin 0. SensESP by default recognizes the following button presses:

- Short press (less than 1 second): Restart the device
- Long press (more than 1 second): Reset the network settings
- Very long press (more than 5 seconds): Reset the device to factory settings

If you want to use a different pin for the button, add the following line to the Builder section in main.cpp, before the `->get_app()` instruction:

```c++
->set_button_pin(15)
```
where the new button GPIO would be 15 in this example.

If you want to disable the button handling completely, add the following line to the Builder section in main.cpp, before the `->get_app()` instruction:

```c++
->set_button_pin(-1)
```

It is also possible to define completely custom button behavior by disabling the default button as above and then adding your own button handler class that inherits from `BaseButtonHandler` or `MinimalButtonHandler`.

## OTA updates

Activating Over The Air (OTA) updates, requires a 2 Step setup:

STEP 1:
1. Add the line

   ```c++
    ->enable_ota("SomeOTAPassword")
   ```

   to the Builder section in main.cpp, before the `->get_app()` instruction. Nothing else.

2. Upload `main.cpp` to the ESP via USB link, which will still work as usual.


STEP 2:
1. Add the following lines to platformio.ini, under the `[env:esp32dev]` section:

   ```ìni
     upload_protocol = espota
     upload_port = "ip address of esp"  (mDNS might not work)
     upload_flags =
         --auth=SomeOTAPassword
   ```

2. Upload `main.cpp` again. This will now happen OTA irrespective of whether USB cable is connected or disconnected.

----

## Network transports

As of the transport-agnostic networking refactor, SensESP can reach
the Signal K server over any interface that Arduino-ESP32 3.x can
bring up, not only WiFi. A single `NetworkStateProducer` owned by
`SensESPApp` subscribes to the unified `Network.onEvent()` bus and
emits a transport-agnostic `NetworkState` whenever any interface's
IP-layer state changes. The HTTP server, mDNS, Signal K websocket
client, and system-info sensors all query a `NetworkProvisioner`
abstract base instead of touching `WiFi` directly, so they work the
same on WiFi and Ethernet.

### WiFi (default)

Existing user sketches do not need to change. `SensESPAppBuilder`
defaults to constructing a `WiFiProvisioner` (the former `Networking`
class — the old name is still available as a deprecated typedef).

```c++
SensESPAppBuilder builder;
auto app = builder.set_hostname("my-device")
                  ->set_wifi_client("SSID", "password")
                  ->get_app();
```

### Ethernet (ESP32-P4)

Native RMII Ethernet is supported on ESP32-P4 boards with an external
PHY. The library currently ships a configuration preset for the
Waveshare ESP32-P4-WIFI6-POE-ETH dev board (IP101GRI PHY, external
25 MHz crystal). To use it:

```c++
#include "sensesp/net/ethernet_provisioner.h"
#include "sensesp_app_builder.h"

SensESPAppBuilder builder;
auto app = builder.set_hostname("my-device")
                  ->set_ethernet(
                      sensesp::EthernetConfig::waveshare_esp32p4_poe())
                  ->get_app();
```

`set_ethernet()` implicitly calls `disable_wifi()` so no WiFi soft-AP
fall-back or captive portal is registered. The SensESP web UI remains
reachable at the DHCP-assigned address on the Ethernet side.

`EthernetProvisioner` is only declared on targets where the
Arduino-ESP32 ETH driver is usable — currently ESP32-P4. Calling
`set_ethernet()` on an unsupported target produces a compile error at
the use site (`EthernetConfig was not declared in this scope`), not a
silent runtime failure. See `examples/p4_ethernet/` for a full
buildable example.

### Writing your own provisioner

Subclass `sensesp::NetworkProvisioner` (see
`sensesp/net/network_provisioner.h`) and bring your interface up
yourself — e.g. PPP over UART for a cellular modem. The base class
only requires `local_ip()`, `gateway_ip()`, `mac_address()`, and
`is_connected()`. Install your factory by calling
`app->set_network_provisioner_factory(...)` before
`SensESPApp::setup()` runs (typically from a builder extension). The
unified `NetworkStateProducer` will pick up state changes automatically
as long as your interface emits the standard `ARDUINO_EVENT_*`
events through `Network.onEvent`.

----

## WiFi manager

SensESP v3 includes a built-in WiFi provisioner — no external WiFiManager library is needed. The device can run in simultaneous AP (Access Point) and STA (Station/client) mode, so you can configure WiFi credentials while the device is also connected to your network.

The AP mode serves a built-in captive portal with the same web UI used for runtime configuration. Up to 3 WiFi client configurations can be saved; if the current network connection fails, the device cycles through the saved networks automatically.

WiFi configuration is persisted to the device file system and restored on restart. If you hard-code credentials with `set_wifi_client()` in your sketch, those serve as defaults — but they can be overridden by saved configuration through the web UI. This means you can ship a device with initial credentials and let the end user change them later without reflashing.

## System info sensors

SensESP provides five built-in system info sensors for monitoring device health:

| Sensor | Type | Description |
|--------|------|-------------|
| `SystemHz` | `float` | Event loop frequency (iterations per second) |
| `FreeMem` | `uint32_t` | Free heap memory in bytes |
| `Uptime` | `float` | Seconds since boot |
| `IPAddrDev` | `String` | Device IP address |
| `WiFiSignal` | `int` | WiFi signal strength (RSSI in dBm) |

The easiest way to enable all of them at once is on the builder:

```c++
SensESPAppBuilder builder;
auto app = builder.set_hostname("my-device")
                  ->enable_system_info_sensors()
                  ->get_app();
```

You can also enable individual sensors if you only need a subset:

```c++
builder.enable_free_mem_sensor()
       ->enable_uptime_sensor()
       ->enable_system_hz_sensor()
       ->enable_ip_address_sensor()
       ->enable_wifi_signal_sensor()
```

Each sensor is a `ValueProducer` that can be connected to Signal K outputs or other transforms, just like any other sensor in your project. The optional prefix parameter controls the Signal K path prefix (defaults to `"sensors.sensesp"`).
