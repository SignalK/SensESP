#ifndef _value_consumer_H_
#define _value_consumer_H_

#include <ArduinoJson.h>
#include <stdint.h>

#include "sensesp.h"

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
   * @param input_channel Consumers can have one or more inputs feeding them.
   *  This parameter allows you to specify which input number the producer
   *  is connecting to. For single input consumers, leave the index at zero.
   */
  virtual void set(T new_value, uint8_t input_channel = 0) {}

  virtual void set_input(T new_value, uint8_t input_channel = 0) {
    static bool warned = false;
    if (!warned) {
      warned = true;
      debugW("set_input() is deprecated. Use set() instead.");
    }
    set(new_value, input_channel);
  }

  /**
   * Registers this consumer with the specified producer, letting it
   * know that this consumer would like to receive notifications whenever
   * its value changes.
   * @param input_channel Consumers can have one or more inputs feeding them.
   *  This parameter allows you to specify which input number the producer
   *  is connecting to. For single input consumers, leave the index at zero.
   */
  void connect_from(ValueProducer<T>* producer, uint8_t input_channel = 0) {
    producer->attach([producer, this, input_channel]() {
      this->set(producer->get(), input_channel);
    });
  }
};

typedef ValueConsumer<float> FloatConsumer;
typedef ValueConsumer<int> IntConsumer;
typedef ValueConsumer<bool> BoolConsumer;
typedef ValueConsumer<String> StringConsumer;

}  // namespace sensesp

#endif
