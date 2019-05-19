#include "wantsenable.h"

std::vector<WantsEnable*> WantsEnable::enableList;

WantsEnable::WantsEnable(uint8_t priority) : priority{priority} {
   WantsEnable::enableList.push_back(this);
}



void WantsEnable::enableAll() {
    for (uint8_t priority = 0; priority <= 10; priority++) {
        for (auto const& pObj : WantsEnable::enableList) {
            if (pObj->priority == priority) {
                pObj->enable();
            }
        }
    }
}
