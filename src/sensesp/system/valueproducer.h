#ifndef SENSESP_SYSTEM_VALUE_PRODUCER_H_
#define SENSESP_SYSTEM_VALUE_PRODUCER_H_

#include <ArduinoJson.h>

#include "observable.h"
#include "valueconsumer.h"

namespace sensesp {

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
  ValueProducer(const T& initial_value) : output_(initial_value) {}

  /**
   * Returns the current value of this producer
   */
  virtual const T& get() const { return output_; }

  /**
   * Connects this producer to the specified consumer, registering that
   * consumer for notifications to when this producer's value changes.
   */
  void connect_to(ValueConsumer<T>* consumer) {
    this->attach([this, consumer]() { consumer->set(this->get()); });
  }
  void connect_to(ValueConsumer<T>& consumer) {
    this->attach([this, consumer]() { consumer.set(this->get()); });
  }

  /**
   * @brief Connect a producer to a consumer of a different type.
   *
   * This allows you to connect a producer to a consumer of a different type.
   * Automatic type conversion is performed.
   *
   * @tparam CT Consumer type
   * @param consumer Consumer object to connect to
   */
  template <typename CT>
  void connect_to(ValueConsumer<CT>* consumer) {
    this->attach([this, consumer]() { consumer->set(CT(this->get())); });
  }
  template <typename CT>
  void connect_to(ValueConsumer<CT>& consumer) {
    this->attach([this, consumer]() { consumer.set(CT(this->get())); });
  }

  /**
   *  If the consumer this producer is connecting to is ALSO a producer
   *  of values of the same type, connect_to() calls can be chained
   *  together, as this specialized version returns the producer/consumer
   *  being conencted to so connect_to() can be called on THAT object.
   */
  template <typename T2>
  Transform<T, T2>* connect_to(Transform<T, T2>* consumer_producer) {
    this->attach([this, consumer_producer]() {
      consumer_producer->set(T(this->get()));
    });
    return consumer_producer;
  }
  template <typename T2>
  Transform<T, T2>* connect_to(Transform<T, T2>& consumer_producer) {
    this->attach(
        [this, consumer_producer]() { consumer_producer.set(T(this->get())); });
    return &consumer_producer;
  }

  /**
   * @brief Connect a producer to a transform with a different input type
   *
   * This allows you to connect a producer to a transform with a different
   * input type. Automatic type conversion is performed.
   *
   * @tparam TT Transform input type
   * @tparam T2 Transform output type
   * @param consumer_producer Transform object to connect to
   * @return Transform<TT, T2>*
   */
  template <typename TT, typename T2>
  Transform<TT, T2>* connect_to(Transform<TT, T2>* consumer_producer) {
    this->attach([this, consumer_producer]() {
      consumer_producer->set(TT(this->get()));
    });
    return consumer_producer;
  }
  template <typename TT, typename T2>
  Transform<TT, T2>* connect_to(Transform<TT, T2>& consumer_producer) {
    this->attach([this, consumer_producer]() {
      consumer_producer->set(TT(this->get()));
    });
    return &consumer_producer;
  }

  /*
   * Set a new output value and notify consumers about it
   */
  void emit(const T& new_value) {
    this->output_ = new_value;
    Observable::notify();
  }

 protected:
  /**
   * The current value of this producer is stored here in this output member
   * (unless descendant classes override ValueProducer::get())
   */
  T output_;
};

typedef ValueProducer<float> FloatProducer;
typedef ValueProducer<int> IntProducer;
typedef ValueProducer<bool> BoolProducer;
typedef ValueProducer<String> StringProducer;

}  // namespace sensesp

#endif
