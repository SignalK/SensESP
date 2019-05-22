#ifndef _transform_H_
#define _transform_H_

#include <set>

#include "Arduino.h"
#include <ArduinoJson.h>

#include "system/configurable.h"
#include "system/observable.h"
#include "system/valueproducer.h"
#include "system/wantsenable.h"
#include "system/signalksource.h"
#include "sensesp.h"


// TODO: Split into multiple files

///////////////////
// Transforms transform raw device readouts into useful sensor values.

/**
 * The base class for all transforms. All transforms are can be subscribed
 * to by calling attach() (inherited from Observable). They can
 * have an optional persistence configuration by specifying an "id" to
 * save the configuration data in.
 */
class TransformBase : public SignalKSource,
                      public Configurable, 
                      public WantsEnable {
 public:
    TransformBase(String sk_path, String id="", String schema="");

  
  // Primary purpose of this was to supply SignalK sources
  // (now handled by SignalKSource::get_sources). Should
  // this be deprecated?
  static const std::set<TransformBase*>& get_transforms() {
    return transforms;
  }
 
  private:
    static std::set<TransformBase*> transforms;
};


/**
 * The main Transform class. A transform is identified primarily by the
 * type of value that is produces (i.e. a Transform<float> is a
 * ValueProducer<float> that generates float values)
 */
template <typename T>
class Transform : public TransformBase, public ValueProducer<T> {
    public:
      Transform(String sk_path, String id="", String schema="") :
         TransformBase(sk_path, id, schema), ValueProducer<T>() {
      }
};


#define BooleanTransform Transform<bool>
#define NumericTransform Transform<float>
#define IntegerTransform Transform<int>
#define StringTransform Transform<String>


/**
 * A OneToOneTransform is a common type of transform that consumes, transforms,
 * then outputs values of the same data type.
 */
template <typename T>
class OneToOneTransform : public ValueConsumer<T>, public Transform<T> {

  public: 
     OneToOneTransform(String sk_path, String id="", String schema="") :
      ValueConsumer<T>(),
      Transform<T>(sk_path, id, schema) {
  }
};

#define OneToOneNumericTransform OneToOneTransform<float>
#define OneToOneIntegerTransform OneToOneTransform<int>
#define OneToOneBooleanTransform OneToOneTransform<bool>
#define OneToOneStringTransform OneToOneTransform<String>

#endif
