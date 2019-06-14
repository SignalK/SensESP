#ifndef _enable_H_
#define _enable_H_

#include <queue>
#include <stdint.h>

/**
 * Classes that implement "Enable" will have their enable() method
 * called automatically at startup when the SensESP app itself
 * is enabled.  The optional priority allows for certain classes
 * to be initialized first. The default priority is zero. 
 * A higher priority (i.e. greater than zero) will be enabled before a 
 * lower one (i.e. less than zero).
 */
class Enable {

    public:
        Enable(uint8_t priority = 0);

        /**
         * Called during the initialization process.  Override this method
         * to add runtime initialization code to your class
         */
        virtual void enable() {}


        const int8_t getEnablePriority() { return priority; }


        void setPriority(uint8_t priority) { this-> priority = priority; }


        /**
         * Called by the SensESP framework to initialize all of the objects
         * marked with this class. They will be initialized in priorty
         * order.
         */
        static void enableAll();
    

        friend bool operator<(const Enable& lhs, const Enable& rhs) { return lhs.priority < rhs.priority; }

    private:
        int8_t priority;

        static std::priority_queue<Enable*> enableList;
};
 

#endif
