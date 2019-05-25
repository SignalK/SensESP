#include "enable.h"

std::priority_queue<Enable*> Enable::enableList;


Enable::Enable(uint8_t priority) : priority{priority} {
   Enable::enableList.push(this);
}


void Enable::enableAll() {
    while (!enableList.empty()) {
        auto& obj = *enableList.top();
        obj.enable();
        enableList.pop();
    } // while
}
