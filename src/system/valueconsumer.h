#ifndef _value_consumer_H_
#define _value_consumer_H_

#include <ArduinoJson.h>
#include <stdint.h>

template <typename T>
class ValueProducer;

/**
 *  A ValueConsumer is any piece of code (like a transformation) that
 *  accepts data for input. They can accept one or more input values
 *  via the set_input() method. They are connected to ValueProducers
 *  via the connect_to() method.
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
  virtual void set_input(T new_value, uint8_t input_channel = 0) {}

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
      this->set_input(producer->get(), input_channel);
    });
  }
  [[deprecated("Use connect_from() instead.")]]
  void connectFrom(
      ValueProducer<T>* producer, uint8_t input_channel = 0) {
    connect_from(producer, input_channel);
  }
};

typedef ValueConsumer<float> NumericConsumer;
typedef ValueConsumer<int> IntegerConsumer;
typedef ValueConsumer<bool> BooleanConsumer;
typedef ValueConsumer<String> StringConsumer;

#endif