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
        debugD("Enabling sensor or transform: %s", obj.getClassName());
        obj.enable();
        enableList.pop();
    } // while
}
