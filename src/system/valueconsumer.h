#ifndef _value_consumer_H_
#define _value_consumer_H_

#include <stdint.h>
#include <ArduinoJson.h>

/**
 *  A ValueConsumer is any piece of code (like a transformation), or a device that
 *  accepts data for input. They can accept one or more input values which are
 *  stored for immediate processing. They are connected to ValueProducers
 *  via the connectTo() method.
 *  @see ValueProducer::connectTo()
 */
template <typename T>
class ValueConsumer {

    public:
        /**
         *  Constructor
         *  @param inputSize optional parameter used to indicate the 
         *    maximum number of input values that this consumer
         *    works with.
         */
        ValueConsumer(uint8_t inputSize = 1) : inputSize{inputSize} {
            inputs = new T[inputSize];
        }

        virtual ~ValueConsumer() {
            delete [] inputs;
        }


        /**
         * Used to set an input of this consumer. It is usually called
         * automatically by a ValueProducer
         * @param newValue the value of the input
         * @param idx The zero based index of the input to this consumer.
         *   Indexes that are out of range will be discarded.
         */
        virtual void set_input(T newValue, uint8_t idx = 0) {

            if (idx >= 0 && idx < inputSize) {
                inputs[idx] = newValue;
                inputUpdated(idx);
            }
        }

        /**
         * Called after each call to set_input() to do additional processing
         * once a new value has been set.
         * @param idex The zero based index of the input that was just changed.
         */
        virtual void inputUpdated(uint8_t idx) {}

    protected:
        T* inputs;
        uint8_t inputSize;

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