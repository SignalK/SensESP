#ifndef _mustenable_H_
#define _mustenable_H_

#include <vector>
#include <stdint.h>

/**
 * Classes that "WantsEnable" will have their enable() method
 * call automatically at startup when the SensESP app itself
 * is enabled.  The optional priority allows for certain classes
 * to be initialized first. A lower priority (e.g. zero) will be
 * enabled for a higher one (e.g. 10). The maximum priority is 10.
 */
class WantsEnable {

    public:
        WantsEnable(uint8_t priority = 10);

        /**
         * Called during the initialization process.  Override this method
         * to add runtime initialization code to your class
         */
        virtual void enable() {}

        /**
         * Called by the SensESP framework to initialize all of the objects
         * marked with this class.
         */
        static void enableAll();
    
    private:
        uint8_t priority;

        static std::vector<WantsEnable*> enableList;
};


#endif
