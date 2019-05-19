#ifndef _value_producer_H_
#define _value_producer_H_

#include "observable.h"
#include <ArduinoJson.h>
#include "valueconsumer.h"

/**
 * A ValueProducer<> is any device or piece of code that outputs a value for consumption
 * elsewhere.  They are Observable, allow code to be notified whenever a new value
 * is available.  They can be connected directly to ValueConsumers of the same type
 * using the connectTo() method.
 */
template <typename T>
class ValueProducer : virtual public Observable {

    public:
        /**
         *  Constructor
         * @param valueIdx Consumers can have one or more inputs feeding them
         *   This parameter allows you to specify which input number this producer
         *   is connecting to. For single input consumers, leave the index at
         *   zero.
         *  @see ValueConsumer::set_input()
         */
        ValueProducer(uint8_t valueIdx = 0) : valueIdx{valueIdx} {}

        virtual T get() { return output; }

        virtual uint8_t getValueIdx() { return valueIdx; }
        virtual void setValueIdx(uint8_t newIdx) { valueIdx = newIdx; }

        void connectTo(ValueConsumer<T>* pConsumer) {
            this->attach([this, pConsumer](){
                pConsumer->set_input(this->get(), this->getValueIdx());
            });
        }

    protected:
        T output;
        uint8_t valueIdx;
};

// The following common types are defined using #define to make the purpose of a template class
// clearer, as well as allow for interoperability between the various classes. If NumericProducer
// inherited from "ValueProducer<float>"" vs just being an alias, it would actually be a different type than
// anything defined as being or inheriting from "ValueProducer<float>"".  When used as an alias, they
// are interchangable.
#define NumericProducer ValueProducer<float> 
#define IntegerProducer ValueProducer<int> 
#define BooleanProducer ValueProducer<bool>
#define StringProducer  ValueProducer<String> 

#endif