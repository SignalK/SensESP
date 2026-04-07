# p4_ethernet

Minimal SensESP application that runs over native RMII Ethernet on a
Waveshare [ESP32-P4-WIFI6-POE-ETH](https://www.waveshare.com/wiki/ESP32-P4-WIFI6-POE-ETH)
dev board. WiFi is disabled entirely — the device brings up the
onboard IP101GRI PHY, runs DHCP, and reaches the Signal K server
over the wired link (optionally powered via PoE through the RJ45
magjack).

This example exists primarily as an end-to-end smoke test for the
transport-agnostic networking refactor
([RFC #849](https://github.com/SignalK/SensESP/issues/849)): it
exercises the new `NetworkProvisioner` / `NetworkStateProducer` path
and the `EthernetProvisioner` concrete implementation from a real
`SensESPAppBuilder` invocation.

## Hardware

- Board: Waveshare ESP32-P4-WIFI6-POE-ETH (ESP32-P4 ECO2 / rev v1.3
  silicon, 16 MB flash, 32 MB PSRAM). Other ESP32-P4 boards with an
  IP101GRI / TLK110-class PHY on the same RMII pinout should also
  work, possibly by extending `EthernetConfig` with a new factory.
- USB-C: the board exposes a single USB-C wired through a CH340
  USB-UART bridge (QinHeng 1a86:55d3) to the P4's UART0 (GPIO 37
  TX / GPIO 38 RX). The P4's native USB-OTG is not exposed, so
  `ARDUINO_USB_CDC_ON_BOOT` is deliberately left undefined —
  Arduino `Serial` maps to UART0 and reaches the host. On recent
  Linux kernels (with `cdc-acm` handling the newer CH340 variant)
  the device enumerates as `/dev/ttyACM0`; older systems loading
  the legacy vendor-specific driver may see `/dev/ttyUSB0` instead.
  On macOS it is usually `/dev/cu.usbmodem*`; on Windows a COM port.
- Ethernet: standard RJ45 patch cable to any switch or router that
  serves DHCP. For isolated bench testing, a Linux host running
  `nmcli connection add type ethernet ifname ethX con-name test
  ipv4.method shared ipv4.addresses 192.168.99.1/24` will spin up a
  NetworkManager-managed `dnsmasq` on that interface.

## Build and flash

This is a standalone PlatformIO project that consumes SensESP from
the parent directory via a `symlink://../..` library dep.

```sh
cd examples/p4_ethernet
pio run -t upload   # PlatformIO auto-detects the serial port
pio device monitor  # view boot log at 115200 bps
```

The `[env:waveshare_esp32p4_eth]` environment in `platformio.ini`
pins pioarduino release 55.03.37 (Arduino-ESP32 3.3.7, ESP-IDF
v5.5.2) and targets the `esp32-p4` board variant with 16 MB flash
override for the Waveshare board's larger SPI flash.

## Expected behaviour

On a successful boot the serial monitor should show:

```text
[p4_eth] boot
...
I (XXX) sensesp/net/ethernet_provisioner.cpp: Bringing up Ethernet ...
I (XXX) sensesp/net/ethernet_provisioner.cpp: Ethernet driver started; ...
...
I (XXX) sensesp/net/network_state.cpp: net_state: Ethernet got IP
[p4_eth] SensESPApp constructed
```

Once DHCP completes, the SensESP web UI should be reachable at
`http://<leased-ip>/`. The status page will show an empty SSID and
RSSI (both WiFi-specific), but `MAC Address` will be populated from
the Ethernet MAC, and the Signal K settings page can be used to
point the device at a running `signalk-server`.

## What this example proves

- `NetworkStateProducer` correctly receives `ARDUINO_EVENT_ETH_GOT_IP`
  through the unified `Network.onEvent` bus and emits
  `kNetworkConnected`.
- `SensESPApp::setup()` dispatches to the builder-supplied factory,
  leaving `wifi_provisioner_` null, and the status page suppresses
  its WiFi-only items.
- `IPAddrDev` (the built-in sensor) reads its value through
  `NetworkProvisioner::local_ip()`, so `sensorDevice.<hostname>.ipAddress`
  on the Signal K server reflects the Ethernet IP without any
  Ethernet-specific code in the sensor.
- `SKWSClient::connect()` gates on `provisioner->is_connected()`
  instead of `WiFi.isConnected()` and successfully dials the Signal
  K server over the wired link.
