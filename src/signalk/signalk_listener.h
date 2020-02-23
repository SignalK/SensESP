#ifndef _signalk_listener_H_
#define _signalk_listener_H_

#include <set>

#include "Arduino.h"
#include <ArduinoJson.h>

#include "system/configurable.h"
#include "system/observable.h"
#include "system/valueproducer.h"
#include "sensesp.h"

/**
 * A SignalK listener is one that listens subscribes for SignalK stream deltas
 * and notifies of value changes
 */
class SKListener : virtual public Observable {

    public:
        /**
         * The constructor
         * @param sk_path The SignalK path that identifies
         * this particular subscription to value
         */
        SKListener(String sk_path, int period);

        /**
         * Returns the current SignalK path.  An empty string
         * is returned if this particular source is not configured
         * or intended to return actual data.
         */
        String& get_sk_path() {
            return sk_path;
        }

        int getPeriod()
        {
            return period;
        }

        virtual void parseValue(JsonObject& json)
        {

        }

        static const std::vector<SKListener*>& get_listeners() {
            return listeners;
        }

    protected:
        String sk_path;

    private:
        static std::vector<SKListener*> listeners;
        int period;
};

#endif
