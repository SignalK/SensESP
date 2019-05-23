#ifndef _observablevalue_H
#define _observablevalue_H

#include "observable.h"
#include "valueproducer.h"

///////////////////
// ObservableValue is simply a value that notifies its observers if
// it gets changed.

template <class T>
class ObservableValue : public ValueProducer<T> {
 public:
  ObservableValue() {}

  ObservableValue(const T& value) {
      ValueProducer<T>::output = value;
  }

  void set(const T& value) {
    ValueProducer<T>::output = value;
    Observable::notify();
  }

  const T& operator=(const T& value) {
      set(value);
      return value;
  }
  
};


#endif