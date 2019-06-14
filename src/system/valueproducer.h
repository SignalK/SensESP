#ifndef _value_producer_H_
#define _value_producer_H_

#include "observable.h"
#include <ArduinoJson.h>
#include "valueconsumer.h"


// The Transform class is defined in transforms/transform.h
template <typename C, typename P> class Transform;


/**
 * A ValueProducer<> is any sensor or piece of code that outputs a value for consumption
 * elsewhere.  They are Observable, allowing code to be notified whenever a new value
 * is available.  They can be connected directly to ValueConsumers of the same type
 * using the connectTo() method.
 */
template <typename T>
class ValueProducer : virtual public Observable {

    public:
        ValueProducer() {}


        /**
         * Returns the current value of this producer
         */
        virtual const T& get() { return output; }



        /**
         * Connects this producer to the specified consumer, registering that
         * consumer for notifications to when this producer's value changes
         * @param inputChannel Consumers can have one or more inputs feeding them
         *   This parameter allows you to specify which input number this producer
         *   is connecting to. For single input consumers, leave the index at
         *   zero.
         *  @see ValueConsumer::set_input()
         */
        void connectTo(ValueConsumer<T>* pConsumer, uint8_t inputChannel = 0) {
            this->attach([this, pConsumer, inputChannel](){
                pConsumer->set_input(this->get(), inputChannel);
            });
        }



        /**
         *  If the consumer this producer is connecting to is ALSO a producer
         *  of values of the same type, connectTo() calls can be chained
         *  together, as this specialized version returns the producer/consumer
         *  being conencted to so connectTo() can be called on THAT object.
         * @param inputChannel Consumers can have one or more inputs feeding them
         *   This parameter allows you to specify which input number this producer
         *   is connecting to. For single input consumers, leave the index at
         *   zero.
         *  @see ValueConsumer::set_input()
         */
        template <typename T2>
        Transform<T, T2>* connectTo(Transform<T, T2>* pConsumerProducer, uint8_t inputChannel = 0) {
            this->attach([this, pConsumerProducer, inputChannel](){
                pConsumerProducer->set_input(this->get(), inputChannel);
            });
            return pConsumerProducer;
        }

    protected:
        /**
         * The current value of this producer is stored here in this output member
         * (unless descendant classes override ValueProducer::get())
         */
        T output;
};


typedef ValueProducer<float> NumericProducer;
typedef ValueProducer<int>  IntegerProducer;
typedef ValueProducer<bool> BooleanProducer;
typedef ValueProducer<String> StringProducer;

#endif