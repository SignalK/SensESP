#ifndef _value_consumer_H_
#define _value_consumer_H_

#include "sensesp.h"

#include <ArduinoJson.h>
#include <stdint.h>

namespace sensesp {

template <typename T>
class ValueProducer;

/**
 *  @brief A base class for piece of code (like a transform) that
 *  accepts data for input. ValueConsumers can accept one or more input values
 *  via the set() method. They are connected to `ValueProducers`
 *  via the `connect_to()` method.
 *  @see ValueProducer::connect_to()
 */
template <typename T>
class ValueConsumer {
 public:
  /**
   * Used to set an input of this consumer. It is usually called
   * automatically by a ValueProducer.
   * @param new_value the value of the input
   */
  virtual void set(T new_value) {}

  virtual void set_input(T new_value) {
    static bool warned = false;
    if (!warned) {
      warned = true;
      debugW("set_input() is deprecated. Use set() instead.");
    }
    set(new_value);
  }

  /**
   * Registers this consumer with the specified producer, letting it
   * know that this consumer would like to receive notifications whenever
   * its value changes.
   */
  void connect_from(ValueProducer<T>* producer) {
    producer->attach([producer, this]() { this->set(producer->get()); });
  }
};

typedef ValueConsumer<float> FloatConsumer;
typedef ValueConsumer<int> IntConsumer;
typedef ValueConsumer<bool> BoolConsumer;
typedef ValueConsumer<String> StringConsumer;

}  // namespace sensesp

#endif
