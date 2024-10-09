#ifndef SENSESP_SYSTEM_VALUE_PRODUCER_H_
#define SENSESP_SYSTEM_VALUE_PRODUCER_H_

#include <ArduinoJson.h>
#include <memory>

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
   * @brief Connect a producer to a transform with a different input type
   *
   * This allows you to connect a producer to a transform with a different
   * input type. Automatic type conversion is performed.
   *
   * @tparam VConsumer ValueConsumer type
   * @tparam CInput Consumer input type
   * @param consumer_producer Transform object to connect to
   * @return std::shared_ptr<VConsumer> The connected consumer
   */
  template <typename VConsumer>
  typename std::enable_if<
    std::is_base_of<ValueConsumer<typename VConsumer::input_type>, VConsumer>::value &&
    std::is_convertible<T, typename VConsumer::input_type>::value,
    std::shared_ptr<VConsumer>
  >::type
  connect_to(std::shared_ptr<VConsumer> consumer) {
    using CInput = typename VConsumer::input_type;
    // Capture consumer_producer as weak_ptr to avoid strong reference cycles
    std::weak_ptr<VConsumer> weak_consumer = consumer;
    this->attach([this, weak_consumer]() {
      if (auto consumer = weak_consumer.lock()) {
        consumer->set(static_cast<CInput>(this->get()));
      }
    });
    return consumer;
  }

  template <typename VConsumer>
  typename std::enable_if<
    std::is_base_of<ValueConsumer<typename VConsumer::input_type>, VConsumer>::value &&
    std::is_convertible<T, typename VConsumer::input_type>::value,
    VConsumer*
  >::type
  connect_to(VConsumer* consumer) {
    using CInput = typename VConsumer::input_type;
    this->attach([this, consumer]() {
      consumer->set(static_cast<CInput>(this->get()));
    });
    return consumer;
  }

  template <typename VConsumer>
  typename std::enable_if<
    std::is_base_of<ValueConsumer<typename VConsumer::input_type>, VConsumer>::value &&
    std::is_convertible<T, typename VConsumer::input_type>::value,
    VConsumer*
  >::type
  connect_to(VConsumer& consumer) {
    return connect_to(&consumer);
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

#endif  // SENSESP_SYSTEM_VALUE_PRODUCER_H_
