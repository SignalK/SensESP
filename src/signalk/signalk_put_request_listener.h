#ifndef _signalk_putrequestlistener_H_
#define _signalk_putrequestlistener_H_

#include <ArduinoJson.h>

#include <set>

#include "Arduino.h"
#include "sensesp.h"
#include "system/configurable.h"
#include "system/observable.h"
#include "system/valueproducer.h"

/**
 * @brief An Obervable class that listens for Signal K PUT
 * requests coming over the websocket connection and notifies
 * observers of the change.
 * @see SKPutRequestListener
 */
class SKPutListener : virtual public Observable {
 public:
  /**
   * The constructor
   * @param sk_path The Signal K path that identifies the signal k path
   * this particular listener responds to
   */
  SKPutListener(String sk_path);

  String& get_sk_path() { return sk_path; }

  virtual void parse_value(JsonObject& put) = 0;

  static const std::vector<SKPutListener*>& get_listeners() { return listeners; }

 protected:
  String sk_path;

 private:
  static std::vector<SKPutListener*> listeners;
  int listen_delay;
};


/**
 * @brief  An object that listens for PUT requests to come in
 * for the specified SignalK path. The value is then emitted for
 * further processing.
 */
template <class T>
class SKPutRequestListener : public SKPutListener, public ValueProducer<T> {
 public:
  SKPutRequestListener(String sk_path)
      : SKPutListener(sk_path) {
    if (sk_path == "") {
      debugE("SKPutRequestListener: User has provided no sk_path to respond to.");
    }
  }

  void parse_value(JsonObject& put) override {
    this->emit(put["value"].as<T>());
  }
};

typedef SKPutRequestListener<float> FloatSKPutRequestListener;
typedef SKPutRequestListener<int> IntSKPutRequestListener;
typedef SKPutRequestListener<bool> BoolSKPutRequestListener;
typedef SKPutRequestListener<String> StringSKPutRequestListener;

#endif
