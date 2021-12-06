---
layout: default
title: Getting started
nav_order: 20
---

# Getting started

- Getting started with an example project
- Supported hardware
- Basic program structure

Pretium quam vulputate dignissim suspendisse in est ante. Urna neque viverra justo nec ultrices. Donec massa sapien faucibus et molestie ac feugiat sed lectus. Elit duis tristique sollicitudin nibh sit amet commodo nulla facilisi. Eget est lorem ipsum dolor sit amet consectetur adipiscing. Urna cursus eget nunc scelerisque viverra mauris in. Nunc vel risus commodo viverra maecenas accumsan lacus vel. Sem nulla pharetra diam sit amet nisl suscipit adipiscing. Eget felis eget nunc lobortis. Pellentesque elit ullamcorper dignissim cras tincidunt lobortis. Elementum nisi quis eleifend quam adipiscing vitae. Nisl condimentum id venenatis a condimentum vitae sapien. Id nibh tortor id aliquet lectus proin nibh nisl condimentum. Semper auctor neque vitae tempus quam pellentesque nec nam aliquam. Tempus imperdiet nulla malesuada pellentesque elit.

```c++
void setup() {
#ifndef SERIAL_DEBUG_DISABLED
  SetupSerialDebug(115200);
#endif

  SensESPAppBuilder builder;
  sensesp_app = builder
    .set_hostname("rpm-example")
    ->set_wifi("my-wifi-ssid", "my-wifi-passphrase")
    ->set_sk_server("my-sk-server.local", 3000)
    ->enable_system_info_sensors()
    ->enable_ota("my-ota-password")
    ->get_app();
}
```
