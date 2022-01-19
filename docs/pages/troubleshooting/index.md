---
layout: default
title: Troubleshooting
nav_order: 80
---

# Troubleshooting

A not-at-all-organized assortment of common problems and solutions.

## Help on Slack

Slack is an online collaboration platform used by developers. You can join in the Signal K discussion on Slack at
http://slack-invite.signalk.org/. The `#sensors` channel is the best place for you to ask questions about SensESP.

## Missing Libraries

SensESP is a complex library and unfortunately, library dependency issues are not uncommon, especially if you last built the project some time ago, because some of the dependencies have changed. The first thing you should _always_ try is deleting your local `.pio` directory within your project directory. That will make PlatformIO re-download and recompile all dependency libraries and is often enough to resolve problems.

If you do that, and then you get an error about a particular library not being found, or not installed, or something like that, it could be that you don't have `git` installed on your computer. Some of the libraries used by SensESP can be installed only from their git repo URL, and without `git`, that won't work.
https://git-scm.com/book/en/v2/Getting-Started-Installing-Git

## Problems Connecting To, or Staying Connected To, Your Network, or Your SK Server

If the Signal K Server on your computer was installed by some other program, like OpenPlotter, it may have made some settings about network sharing, or some other settings, that just don't work with SensESP in certain situations.

If the Pi that's running Signal K and OpenPlotter is "headless" (i.e., it doesn't have its own keyboard and monitor), the only way to get to Signal K (so that you can, for example, authorize a SensESP security request) is by connecting to a network with internet connectivity. To do that, you need to temporarily change the OpenPlotter network settings as detailed below, do whatever you're going to do in Signal K, and then change the OpenPlotter network settings back to the way they were.

If the Pi is to be permanently connected to a network with an ethernet cable, you will still need to follow the steps below to enable the ethernet setting, but you probably won’t need to revert back to the original settings, because Signal K will be using the wired connection for anything requiring the internet or external communication, while SensESP devices will communicate with the server using the wifi SSID created by OpenPlotter. But if the Pi is connected to your network solely via wifi, that's where the problem lies, and you'll have to follow the steps below. You will need to carry out the setting changes by temporarily connecting your Pi to a monitor, keyboard, and mouse, as you will lose VNC Viewer control temporarily when the settings are adjusted.

1. Connect the Pi to a monitor, keyboard, and mouse.

2. Start OpenPlotter, then select "Network” , and make sure you're on the "Access Point" tab.

3. Before you change anything, make a note of ALL the settings - you will need them later.

4. Select "aa:bb:cc:dd:ee:ff AP and Station" from the "AP" dropdown menu.

5. If your Pi is connected to your network with an ethernet cable, check the checkbox "Add ethernet port to the AP". This should allow Signal K internet connectivity through the wired connection, even while you're connecting to OpenPlotter via wifi.

6. Make sure the "Sharing internet device" dropdown (on the right side of the screen) is set to "auto". This will tell it to share the ethernet connection if there is one (step 5), and to share the wifi connection if there isn't. If it wasn't already set to "auto", click the "Update Sharing" button.

7. If this is the first time configuration of OpenPlotter, you should see the default SSID `openplotter` and default password `12345678`. You should probably change these. (If you've already been through this before, your SSID and password should already be showing.)

8. Click the "Save" button in the lower right corner. The Pi will restart.

9. If the Pi is connected to your network with an ethernet cable, it should now be connected that way, and you should see an "up arrow / down arrow" icon in the very upper right corner of the screen, indicating that you're connected to the network. As long as the Pi will always remain connected with the ethernet cable, you should be able to leave all of the above settings as they are. You should be able to access anything on the Pi, including Signal K and OpenPlotter.

10. If the Pi connects to your network via wifi, you may not be connected to the network yet, so click on the grayed-out "up arrow / down arrow" icon in the upper right corner to make the wifi connection. A list of detected networks will be shown - select the one you want.

11. Once the Pi is connected to the network and internet, you should be able to start Signal K to do whatever you need to do (install a plug-in, authorize a security request, etc.).

12. When finished in Signal K, if the Pi is not connected to the network with an ethernet cable, you will probably need to undo all the changes you made above. Change everything back to how you found it in Step 3 above.

(Thanks to @Graham Kendall for doing all the testing and documentation of the above!)

## Can't Get Past Token Authorization

If you see something like this, repeating over and over in your Serial Monitor, try a Device Reset. If SensESP gets to this point, you should be able to connect to it by going to its IP address (identified on the fourth line below) in a browser. That will bring up the device's menu, and "Reset device" is the last item on that menu. Choose it, and if necessary, restart your device with its physical Reset button. Keep watching the Serial Monitor - if you didn't hard-code your wifi SSID and password in `main.cpp`, you will have to go through the steps to configure those things again, and you'll definitely need to authorize a new access request from the SK Server.

```text
(I) (connect)(C1) WSClient websocket connect attempt (state=0)
(I) (connect)(C1) WiFi is disconnected. SignalK client connection will connect when WiFi is connected.
(I) (wifi_station_connected)(C1) Connected to wifi, SSID: WiFi-2.4-7C4D (signal: -45)
(I) (wifi_station_connected)(C1) IP address of Device: 192.168.1.25
(I) (connect)(C1) WSClient websocket connect attempt (state=0)
(D) (connect)(C1) Initiating websocket connection with server...
(D) (connect)(C1) Websocket is connecting to Signal K server on address 192.168.1.5:3000
(D) (test_token)(C1) Testing token with url http://192.168.1.5:3000/signalk/
(D) (test_token)(C1) Testing resulted in http status 200
(D) (test_token)(C1) Returned payload (length 249) is:
(D) (test_token)(C1) {"endpoints":{"v1":{"version":"1.37.6","signalk-http":"http://192.168.1.5:3000/signalk/v1/api/","signalk-ws":"ws://192.168.1.5:3000/signalk/v1/stream","signalk-tcp":"tcp://192.168.1.5:8375"}},"server":{"id":"signalk-server-node","version":"1.37.6"}}
(D) (test_token)(C1) End of payload output
(D) (test_token)(C1) Attempting to connect to Signal K Websocket...
(I) (connect)(C1) WSClient websocket connect attempt (state=0)
(D) (connect)(C1) Initiating websocket connection with server...
```

etc, etc, etc.

At the time of this writing, it isn't known exactly what's happening or why; only that a Device Reset seems to clear it up.

Thanks to @Sailabout on Slack for documenting this!
