#ifndef _signalk_emitter_H_
#define _signalk_emitter_H_

#include <set>

#include "Arduino.h"
#include <ArduinoJson.h>

#include "system/configurable.h"
#include "system/observable.h"
#include "system/valueproducer.h"
#include "sensesp.h"

/**
 * A SignalK emitter is one that produces SignalK output to be
 * forwarded to the SignalK server (if the system is connected
 * to one).  SignalK is reported via the as_signalK() method.
 */
class SKEmitter : virtual public Observable {

    public:
        /**
         * The constructor
         * @param sk_path The SignalK path that identifies
         * this particular output
         */
        SKEmitter(String sk_path);


        /**
         * Returns the data to be reported to the server as
         * a SignalK json string.
         */
        virtual String as_signalK() { return "not implemented"; }


        /**
         * Returns the current SignalK path.  An empty string
         * is returned if this particular source is not configured
         * or intended to return actual data.
         */
        String& get_sk_path() {
            return sk_path;
        }

        void set_sk_path(const String& path) {
            sk_path = path;
        }

        static const std::vector<SKEmitter*>& get_sources() {
            return sources;
        }

    protected:
        String sk_path;

    private:
        static std::vector<SKEmitter*> sources;

};

#endif
