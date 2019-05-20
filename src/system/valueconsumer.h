#ifndef _value_consumer_H_
#define _value_consumer_H_

#include <stdint.h>
#include <ArduinoJson.h>

/**
 *  A ValueConsumer is any piece of code (like a transformation) or device that
 *  accepts data for input. They can accept one or more input values 
 *  via the set_input() method. They are connected to ValueProducers
 *  via the connectTo() method.
 *  @see ValueProducer::connectTo()
 */
template <typename T>
class ValueConsumer {

    public:
        /**
         * Used to set an input of this consumer. It is usually called
         * automatically by a ValueProducer
         * @param newValue the value of the input
         * @param idx The zero based index of the input to this consumer.
         */
        virtual void set_input(T newValue, uint8_t idx = 0) {
        }

};

// The following common types are defined using #define to make the purpose of a template class
// clearer, as well as allow for interoperability between the various classes. If NumericConsumer
// inherited from "ValueConsumer<float>"" vs just being an alias, it would actually be a different type than
// anything defined as being or inheriting from "ValueConsumer<float>"".  When used as an alias, they
// are interchangable.
#define NumericConsumer ValueConsumer<float> 
#define IntegerConsumer ValueConsumer<int> 
#define BooleanConsumer ValueConsumer<bool>
#define StringConsumer ValueConsumer<String> 

#endif