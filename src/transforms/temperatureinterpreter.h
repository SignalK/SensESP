#ifndef _temperatureinterpreter_H
#define _temperatureinterpreter_H

#include "interpolate.h"

/**
 * This transform takes a resistance value on Ohms (usually from a sender) and returns the estimated 
 * temperature in Kelvin.  Default data is based on samples taken from a Westerbeke generator
 * temperature gauge.
 */
class TemperatureInterpreter : public Interpolate {

    public:
        TemperatureInterpreter(String sk_path="", String config_id="", String schema="");
};

#endif
