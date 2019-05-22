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

  ObservableValue(T value) {
      ValueProducer<T>::output = value;
  }

  void set(T value) {
    ValueProducer<T>::output = value;
    Observable::notify();
  }

  T operator=(T value) {
      set(value);
      return value;
  }
  
};


#endif