#ifndef _value_producer_H_
#define _value_producer_H_

#include <ArduinoJson.h>

#include "observable.h"
#include "valueconsumer.h"

// The Transform class is defined in transforms/transform.h
template <typename C, typename P>
class Transform;

/**
 * @brief A base class for any sensor or piece of code that outputs a value for
 * consumption elsewhere.  
 * <p>They are Observable, allowing code to be notified
 * whenever a new value is available.  They can be connected directly to
 * `ValueConsumer`s of the same type using the `connect_to()` method.
 * @see ValueConsumer
 */
template <typename T>
class ValueProducer : virtual public Observable {
 public:
  ValueProducer() {}

  /**
   * Returns the current value of this producer
   */
  virtual const T& get() const { return output; }

  /**
   * Connects this producer to the specified consumer, registering that
   * consumer for notifications to when this producer's value changes
   * @param input_channel Consumers can have one or more inputs feeding them
   *   This parameter allows you to specify which input number this producer
   *   is connecting to. For single input consumers, leave the index at
   *   zero.
   *  @see ValueConsumer::set_input()
   */
  void connect_to(ValueConsumer<T>* consumer, uint8_t input_channel = 0) {
    this->attach([this, consumer, input_channel]() {
      consumer->set_input(this->get(), input_channel);
    });
  }
  
  /**
   *  If the consumer this producer is connecting to is ALSO a producer
   *  of values of the same type, connect_to() calls can be chained
   *  together, as this specialized version returns the producer/consumer
   *  being conencted to so connect_to() can be called on THAT object.
   * @param input_channel Consumers can have one or more inputs feeding them
   *   This parameter allows you to specify which input number this producer
   *   is connecting to. For single input consumers, leave the index at
   *   zero.
   *  @see ValueConsumer::set_input()
   */
  template <typename T2>
  Transform<T, T2>* connect_to(Transform<T, T2>* consumer_producer,
                               uint8_t input_channel = 0) {
    this->attach([this, consumer_producer, input_channel]() {
      consumer_producer->set_input(this->get(), input_channel);
    });
    return consumer_producer;
  }

  /*
   * Set a new output value and notify consumers about it
   */
  void emit(T new_value) {
    this->output = new_value;
    Observable::notify();
  }

 protected:
  /**
   * The current value of this producer is stored here in this output member
   * (unless descendant classes override ValueProducer::get())
   */
  T output;
};

typedef ValueProducer<float> FloatProducer;
typedef ValueProducer<int> IntProducer;
typedef ValueProducer<bool> BoolProducer;
typedef ValueProducer<String> StringProducer;

#endif