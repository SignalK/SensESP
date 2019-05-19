#include "mustenable.h"

std::vector<MustEnable*> MustEnable::enableList;

MustEnable::MustEnable(uint8_t priority) : priority{priority} {
   MustEnable::enableList.push_back(this);
}



void MustEnable::enableAll() {
    for (uint8_t priority = 0; priority <= 10; priority++) {
        for (auto const& pObj : MustEnable::enableList) {
            if (pObj->priority == priority) {
                pObj->enable();
            }
        }
    }
}
