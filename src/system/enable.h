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


        void setClassName(const char* newClassName) { className = newClassName; }
        
        
        virtual const char* getClassName() { return className; };


        const uint8_t getEnablePriority() { return priority; }


        void setPriority(uint8_t priority) { this-> priority = priority; }


        /**
         * Called by the SensESP framework to initialize all of the objects
         * marked with this class. They will be initialized in priorty
         * order. If you want to see see the name of each sensor and transport
         * in the serial monitor as each one is enabled, add the following
         * to your project's platformio.ini file:
         * 
         * build_unflags = -fno-rtti
         */
        static void enableAll();
    

        friend bool operator<(const Enable& lhs, const Enable& rhs) { return lhs.priority < rhs.priority; }

    
    protected:
        const char* className = "Enable";
    
    
    private:
        uint8_t priority;

        static std::priority_queue<Enable*> enableList;
};
 

#endif
