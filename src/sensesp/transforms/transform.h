#ifndef SENSESP_TRANSFORMS_TRANSFORM_H_
#define SENSESP_TRANSFORMS_TRANSFORM_H_

#include "sensesp.h"

#include <ArduinoJson.h>
#include <set>

#include "sensesp/system/configurable.h"
#include "sensesp/system/observable.h"
#include "sensesp/system/valueconsumer.h"
#include "sensesp/system/valueproducer.h"

namespace sensesp {

// TODO(mairas): Split into multiple files

///////////////////
// Transforms transform raw device readouts into useful sensor values.

/**
 * @brief The base class for all transforms. A transforms takes a value
 * in, transforms it in some way, and outputs the transformed value.
 *
 * All transforms are subscribed
 * to by calling attach() (inherited from Observable).
 *
 * @param config_path The path to configure the Transform in the Config UI.
 */
class TransformBase : public Configurable {
 public:
  TransformBase(const String& config_path = "");

  // Primary purpose of this was to supply Signal K sources
  // (now handled by SKEmitter::get_sources). Should
  // this be deprecated?
  static const std::set<TransformBase*>& get_transforms() {
    return transforms_;
  }

 private:
  static std::set<TransformBase*> transforms_;
};

/**
 * @brief The main Transform class. A transform is identified primarily by the
 * type of value that is produces (i.e. a Transform<float> is a
 * ValueProducer<float> that generates float values)
 */
template <typename C, typename P>
class Transform : public TransformBase,
                  public ValueConsumer<C>,
                  public ValueProducer<P> {
 public:
  Transform(String config_path = "")
      : TransformBase(config_path), ValueConsumer<C>(), ValueProducer<P>() {}

  /**
   * A convenience method that allows up to five producers to be
   * quickly connected to the input of the ValueConsumer side of this
   * transform. The first producer will be connected to input
   * channel zero, the second one to input channel 1, etc.
   * "this" is returned, which allows the ValueProducer side
   * of this transform to then be wired to other transforms via
   * a call to connect_to().
   */
  Transform<C, P>* connect_from(ValueProducer<P>* producer0,
                                ValueProducer<P>* producer1 = NULL,
                                ValueProducer<P>* producer2 = NULL,
                                ValueProducer<P>* producer3 = NULL,
                                ValueProducer<P>* producer4 = NULL) {
    this->ValueConsumer<C>::connect_from(producer0);
    if (producer1 != NULL) {
      this->ValueConsumer<C>::connect_from(producer1, 1);
    }
    if (producer2 != NULL) {
      this->ValueConsumer<C>::connect_from(producer2, 2);
    }
    if (producer3 != NULL) {
      this->ValueConsumer<C>::connect_from(producer3, 3);
    }
    if (producer4 != NULL) {
      this->ValueConsumer<C>::connect_from(producer4, 4);
    }
    return this;
  }
};

/**
 * @brief A common type of transform that consumes,
 * transforms, then outputs values of the same data type.
 */
template <typename T>
class SymmetricTransform : public Transform<T, T> {
 public:
  SymmetricTransform(String config_path = "") : Transform<T, T>(config_path) {}
};

typedef SymmetricTransform<float> FloatTransform;
typedef SymmetricTransform<int> IntegerTransform;
typedef SymmetricTransform<bool> BooleanTransform;
typedef SymmetricTransform<String> StringTransform;

}  // namespace sensesp

#endif
