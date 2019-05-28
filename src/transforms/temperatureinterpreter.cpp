#include "temperatureinterpreter.h"

#include <RemoteDebug.h>

TemperatureInterpreter::TemperatureInterpreter(String sk_path, String config_id, String schema) :
    Interpolate(sk_path, NULL, config_id, schema) {

   // Lookup table for Ohms to Kelvin
   clearSamples();
   addSample(Interpolate::Sample(0, 418.9));
   addSample(Interpolate::Sample(5, 414.71));
   addSample(Interpolate::Sample(36, 388.71));
   addSample(Interpolate::Sample(56, 371.93));
   addSample(Interpolate::Sample(59, 366.48));
   addSample(Interpolate::Sample(81, 355.37));
   addSample(Interpolate::Sample(112, 344.26));
   addSample(Interpolate::Sample(240, 322.04));
   addSample(Interpolate::Sample(550, 255.37));
   addSample(Interpolate::Sample(10000, 237.6));
}
