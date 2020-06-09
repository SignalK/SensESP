#ifndef _app_options_H_
#define _app_options_H_
#include "sensesp_app.h"
#include "sensesp.h"

class SensESPAppOptions {
    public:
        SensESPAppOptions* useDefault();
        SensESPAppOptions* useWifi(String ssid, String password);
        SensESPAppOptions* useServer(String serverAddress, int port);
        SensESPAppOptions* useStandardSensors(StdSensors_t sensors);
        SensESPAppOptions* searchForServer();


        String getSsid() { return ssid; }
        String getPassword() { return password; }
        String getServerAddress() { return serverAddress; }
        int getServerPort() { return serverPort; }
        StdSensors_t getStandardSensors { return sensors;} 
    private:
        String ssid;
        String password;
        String serverAddress;
        int serverPort;
        StdSensors_t sensors;
};

#endif