#ifndef _analog_voltage_H
#define _analog_voltage_H

#include "transforms/linear.h"

/**
 * A Transform that translates the values that comes out of an AnalogInput into a voltage
 */
class AnalogVoltage : public Linear {

    public:
       AnalogVoltage(String config_path = "") : 
          Linear(1.0 / 1024.0 * 3.3, 0, config_path) {
             className = "AnalogVoltage";
          }
};

#endif