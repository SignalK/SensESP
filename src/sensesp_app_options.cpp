#include "sensesp_app_options.h"

SensESPAppOptions::SensESPAppOptions()
{
    useDefault();
}

SensESPAppOptions* SensESPAppOptions::useDefault()
{
    ssid = "";
    password = "";
    serverAddress = "";
    serverPort = 0;
    enabledSensors = all;
    mDNSEnabled = true;
    wifiSet = false;
    serverSet = false;
    ledPin = LED_BUILTIN;
    hostName = "sensesp";

    useLED();

    return this;
}

SensESPAppOptions* SensESPAppOptions::useWifi(String ssid, String password)
{
    this->ssid = ssid;
    this->password = password;
    wifiSet = true;

    return this;
}

SensESPAppOptions* SensESPAppOptions::useServer(String address, int port)
{
    serverAddress = address;
    serverPort = port;
    mDNSEnabled = false;
    serverSet = true;

    return this;
}


SensESPAppOptions* SensESPAppOptions::useStandardSensors(StandardSensorsOptions_t sensors)
{
    enabledSensors = sensors;
    return this;
}

SensESPAppOptions* SensESPAppOptions::useLED(bool enabled, int webSocketConnected, int wifiConnected, int offline)
{
    enableLED = enabled;
    ledIntervals.wifiConnected = wifiConnected;
    ledIntervals.websocketConnected = webSocketConnected;
    ledIntervals.offlineInterval = offline;

    return this;
}

SensESPAppOptions* SensESPAppOptions::useHostName(String hostName)
{
    this->hostName = hostName;
    return this;
}

SensESPAppOptions* SensESPAppOptions::useMDNS()
{
    this->serverAddress = "";
    this->serverPort = 0;
    return this;
}

