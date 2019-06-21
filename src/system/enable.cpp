#include "enable.h"
#include "sensesp_app.h"

std::priority_queue<Enable*> Enable::enableList;


Enable::Enable(uint8_t priority) : priority{priority} {
   Enable::enableList.push(this);
}


const char* Enable::getClassName() { 
    static int counter = 1;
    #if __GXX_RTTI
    return typeid(*this).name();
    #else
    char s[4];
    sprintf(s, "%d", counter++);
    return s;
    #endif
}


void Enable::enableAll() {
    debugD("Enabling all required sensors and transforms");
    while (!enableList.empty()) {
        auto& obj = *enableList.top();
        debugD("Enabling sensor or transform: %s", obj.getClassName());
        obj.enable();
        enableList.pop();
    } // while
    #if ! __GXX_RTTI
    debugD("To see the name of each sensor and transform, add 'build_unflags = -fno-rtti' to platformio.ini");
    #endif
}
