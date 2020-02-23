#ifndef _sk_value_listener_H
#define _sk_value_listener_H

#include "signalk_listener.h"
#include "system/observable.h"
#include "system/valueproducer.h"
#include <ArduinoJson.h>
///////////////////
// SKListener is object that listens for specific value in Signal K, period is interval between updates in ms
///////////////////
template <class T>
class SKValueListener : public SKListener, public ValueProducer<T> {
 public:
  SKValueListener(String sk_path, int period) : SKListener(sk_path, period) 
  {

  }

  void parseValue(JsonObject& json) override
  {
       this->output = (T)json["value"];
       this->notify();
  }  


};


#endif