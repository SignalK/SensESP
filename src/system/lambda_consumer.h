#ifndef _lambda_consumer_H_
#define _lambda_consumer_H_

#include "valueconsumer.h"

/**
 * @brief Construct a new ValueConsumer to execute a single function
 *
 * LambdaConsumer provides an easy way of calling a function based on
 * the output of any ValueProducer. You create the function in
 * main.cpp as a lambda expression, and then pass it as the "function"
 * parameter to the LambaConsumer.
 * See https://github.com/SignalK/SensESP/blob/master/examples/lambda_consumer.cpp
 * for a detailed example.
 *
 * @tparam IN Consumer function input value type
 * 
 * @param function A pointer to the function that's being called.
 * */

template <class IN>
class LambdaConsumer : public ValueConsumer<IN> {
 public:
  LambdaConsumer(std::function<void (IN)> function)
      : ValueConsumer<IN>(), function{function} {
  }

  void set_input(IN input, uint8_t input_channel = 0) override {
    function(input);
  }
  
 protected:
  std::function<void (IN)> function;  
};

#endif
