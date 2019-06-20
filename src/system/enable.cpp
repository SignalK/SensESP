#include "enable.h"
#include "sensesp_app.h"

std::priority_queue<Enable*> Enable::enableList;


Enable::Enable(uint8_t priority) : priority{priority} {
   Enable::enableList.push(this);
}


void Enable::enableAll() {
    debugD("Enabling all required sensors and transforms");
    while (!enableList.empty()) {
        auto& obj = *enableList.top();
        #if __GXX_RTTI
        debugD("Enabling sensor or transform: %s", obj.getClassName());
        #endif
        obj.enable();
        enableList.pop();
    } // while
    #if ! __GXX_RTTI
    debugD("To see each sensor and transform listed here, add 'build_unflags = -fno-rtti' to platformio.ini");
    #endif
}
