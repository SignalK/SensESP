#ifndef _sk_value_listener_H
#define _sk_value_listener_H

#include "signalk_listener.h"
#include "system/observable.h"
#include "system/valueproducer.h"
#include <ArduinoJson.h>
///////////////////
// SKListener is object that listens for specific value in Signal K, listen_Delay is minimum interval between updates in ms
///////////////////
template <class T>
class SKValueListener : public SKListener, public ValueProducer<T> {
 public:
  SKValueListener(String sk_path, int listen_Delay = 1000) : SKListener(sk_path, listen_Delay) 
  {
       if(sk_path == "")
       {
            debugE("SKValueListener: User has provided no sk_path to listen to.");
       }
  }

  void parseValue(JsonObject& json) override
  {
       this->output = (T)json["value"];
       notify();
  }  


};

typedef SKValueListener<float> SKNumericListener;
typedef SKValueListener<int> SKIntListener;
typedef SKValueListener<bool> SKBoolListener;
typedef SKValueListener<String> SKStringListener;
#endif