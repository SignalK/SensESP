# p4_bluedroid_poc

Stage-1 SensESP application that runs over native RMII Ethernet on a
Waveshare [ESP32-P4-WIFI6-POE-ETH](https://www.waveshare.com/wiki/ESP32-P4-WIFI6-POE-ETH)
dev board, built via the new `[env:espidf_esp32p4]` env in the root
`platformio.ini` and the companion `framework = espidf, arduino` build
mode. WiFi is disabled entirely — the device brings up the onboard
IP101GRI PHY, runs DHCP, and reaches the Signal K server over the
wired link.

This example is a standalone PlatformIO project (not an env in the
root `platformio.ini`) because it needs its own `sdkconfig.defaults`,
`idf_component.yml`, and partition table. It is the first end-to-end
demonstration of SensESP running under `framework = espidf, arduino`
on ESP32-P4 hardware with **Bluedroid** selected as the BT host stack
instead of the NimBLE-only Arduino-ESP32 prebuilt libs.

## What this example is — and isn't

This is the **stage-1 baseline**: it stands up SensESP + Ethernet on
a Bluedroid-configured firmware and verifies the whole build and boot
path works. Bluedroid is compiled into the binary (via
`sdkconfig.defaults`) but no BLE code is called from `main.cpp` — the
firmware's only runtime behaviour is the usual SensESP stack plus a
5-second heartbeat log.

The **stage-2 BLE scanner** — which would add a Bluedroid GAP scan
loop over the esp_hosted VHCI transport to the onboard ESP32-C6
companion — is deliberately not included in this commit. During the
PoC work we confirmed that the Bluedroid stack initialises cleanly on
P4 via this build path (`esp_bluedroid_init` / `_enable` return OK,
`esp_ble_gap_set_ext_scan_params` and `esp_ble_gap_start_ext_scan`
both succeed), but advertising reports never arrive from the C6
slave. The same silence was reproduced earlier with the NimBLE host
stack on two separate boards, which strongly suggests the blocker is
in the ESP-Hosted C6 slave firmware (the Arduino-ESP32-distributed
`esp32c6-v2.11.6.bin`), not in the P4 host stack.

Since that is a downstream issue best tracked against
[esp-hosted-mcu](https://github.com/espressif/esp-hosted-mcu) rather
than SensESP, stage-2 is left as a follow-up. The scaffolding in this
example makes it a one-file edit to add the scanner once the slave-fw
story resolves.

## Hardware

- Board: Waveshare ESP32-P4-WIFI6-POE-ETH (ESP32-P4 ECO2 / rev v1.3
  ES silicon, 16 MB flash, 32 MB PSRAM, onboard ESP32-C6 over SDIO).
  The `sdkconfig.defaults` in this directory explicitly selects
  `CONFIG_ESP32P4_SELECTS_REV_LESS_V3=y` because ES silicon uses a
  different startup code path than the rev 3.01+ default — without
  this knob the firmware crashes with an `Illegal Instruction` panic
  during very early boot. See the comment at the top of
  `sdkconfig.defaults` for the details.
- USB-C: same CH340 pass-through to P4 UART0 (GPIO 37/38) as the
  `p4_ethernet` example. Enumerates as `/dev/ttyACM0` on Linux with
  recent `cdc-acm`, `/dev/cu.usbmodem*` on macOS.
- Ethernet: RJ45 to any DHCP-serving switch or router. For isolated
  bench testing on Linux:

  ```sh
  nmcli connection add type ethernet ifname <iface> con-name test \
      ipv4.method shared ipv4.addresses 192.168.99.1/24
  ```

## Build and flash

```sh
cd examples/p4_bluedroid_poc
pio run -t upload
pio device monitor
```

The example's `platformio.ini` uses `framework = espidf, arduino` on
pioarduino 55.03.37 and pulls SensESP from the parent directory via
`symlink://../..`. On first configure, PlatformIO + ESP-IDF will
fetch the `espressif/esp_hosted` managed component and build the
full IDF tree from source — expect a multi-minute first build.
Subsequent builds are incremental.

## Expected behaviour

On a successful boot the serial monitor shows the usual IDF banner
followed by:

```text
I (...) app_init: Project name:     p4_bluedroid_poc
I (...) efuse_init: Chip rev:         v1.3
I (...) main_task: Started on CPU0
I (...) main_task: Calling app_main()
I (...) main_task: Returned from app_main()
I (...) filesystem.cpp: Filesystem initialized
I (...) ethernet_provisioner.cpp: Bringing up Ethernet (PHY type=2 addr=1 ...)
I (...) eth_prov: Ethernet hostname set to "signalk-bluedroid-poc" before DHCP DISCOVER
I (...) esp_netif_handlers: eth0 ip: 192.168.x.y, mask: 255.255.255.0, gw: 192.168.x.1
I (...) net_state: Ethernet got IP
I (...) POC: alive — uptime=7s heap=456151
```

The SensESP web UI should be reachable at
`http://signalk-bluedroid-poc.local/` (or the leased IP directly).
OTA flashing is enabled on port 3232 with password `bluedroid-poc-ota`.

## What this example proves

- The `[env:espidf_esp32p4]` build path in the root `platformio.ini`
  works end-to-end against SensESP as a library dependency.
- The project-owned `sdkconfig.defaults` cleanly selects Bluedroid
  instead of NimBLE and enables the esp_hosted Bluedroid VHCI
  transport, without any workarounds or in-place rewrites of the
  Arduino-ESP32 prebuilt libs.
- SensESP's transport-agnostic networking layer, Ethernet
  provisioner, hostname-in-ETH_START handler, mDNS responder, HTTP
  server, OTA server, and SK websocket client all run cleanly on
  ESP32-P4 in this build mode.
- The 16 MB partition table leaves ample headroom (stage-1 firmware
  is ~23.5% of the 6.5 MB app slot) for adding BLE gateway code, I2C
  sensors, and other SensESP functionality on top.

## Known issue — C6 slave firmware BLE adv forwarding

See the discussion above in "What this example is — and isn't". TL;DR:
the Arduino-ESP32-distributed `esp32c6-v2.11.6.bin` esp_hosted slave
firmware does not appear to forward LE Advertising Reports over HCI
to the P4 host, under either NimBLE or Bluedroid. Track any progress
on that upstream in the
[esp-hosted-mcu issue tracker](https://github.com/espressif/esp-hosted-mcu/issues).
