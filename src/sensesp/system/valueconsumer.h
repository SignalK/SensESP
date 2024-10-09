#ifndef SENSESP_SYSTEM_VALUECONSUMER_H_
#define SENSESP_SYSTEM_VALUECONSUMER_H_

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
   using input_type = T;
  /**
   * Used to set an input of this consumer. It is called
   * automatically by a ValueProducer but can also be called
   * manually.
   *
   * @param new_value the value of the input
   */
  virtual void set(const T& new_value) {}

  [[deprecated("Use set() instead")]]
  virtual void set_input(const T& new_value) {
    set(new_value);
  }

  // Pointer safety cannot be guaranteed because we don't know whether
  // "this" is a shared pointer.
  [[deprecated("Use ValueProducer<T>::connect_to instead")]]
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
