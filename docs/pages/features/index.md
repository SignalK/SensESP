---
layout: default
title: Features
nav_order: 40
---

# Features

- OTA updates

Activating Over The Air (OTA) updates, requires a 2 Step setup:

STEP 1:
1.	Add the line 

     ->enable_ota("SomeOTAPassword") 

to the Builder section in main.cpp, before the ->get_app() instruction. Nothing else.

2.	Upload main.cpp to the ESP via USB link, which will still work as usual. 


STEP 2:
1.	Add the following lines to platformio.ini, under the [env:esp32dev] section: 

     -	upload_protocol = espota
     -	upload_port =  "ip address of esp"  (mDNS might not work)
     -	upload_flags =
          --auth=SomeOTAPassword

2.	Upload main.cpp again. This will now happen OTA irrespective of whether USB cable is connected or disconnected.
 


- WiFi manager
- System info sensors
