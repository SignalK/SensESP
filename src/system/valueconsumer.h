#ifndef _value_consumer_H_
#define _value_consumer_H_

#include <stdint.h>
#include <ArduinoJson.h>

template <typename T> class ValueProducer;

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
         * @param inputChannel The zero based index of the input channel
         *   the new value corresponds to.
         */
        virtual void set_input(T newValue, uint8_t inputChannel = 0) {
        }

        /**
         * Registers this consumer with the specified producer, letting it
         * know that this consumer would like to receive notifications whenever
         * its value changes
         * @param inputChannel The zero based channel number that the producer's
         *   input changes should be reported on.
         */
        void connectFrom(ValueProducer<T>* pProducer, uint8_t inputChannel = 0) {
            pProducer->attach([pProducer, this, inputChannel](){
                this->set_input(pProducer->get(), inputChannel);
            });
        }

};


typedef ValueConsumer<float> NumericConsumer;
typedef ValueConsumer<int> IntegerConsumer;
typedef ValueConsumer<bool> BooleanConsumer;
typedef ValueConsumer<String> StringConsumer;

#endif