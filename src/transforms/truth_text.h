#ifndef _truth_text_H_
#define _truth_text_H_

#include "transforms/transform.h"



/**
 * Returns TRUE if value represents one of truth values recognized by TextToTruth
 */
extern bool textIsTrue(String value);


/**
 * Returns TRUE if value represents one of boolean values recognized by TextToTruth as "false"
 */
extern bool textIsFalse(String value);


/**
 * TextToTruth consumes a string, and outputs boolean or numeric "true" 
 * if the text represents a human readable version of "true". The following
 * case-insensitive values are considered true:
 * <p><ul>
 *   <li>True
 *   <li>T
 *   <li>Yes
 *   <li>Y
 *   <li>On
 *   <li>Any string that represents a non-zero number
 * </ul>
 * 
 */
template <class OUT>
class TextToTruth : public Transform<String, OUT> {

  public:
    virtual void set_input(String input, uint8_t input_channel = 0) override;

};


/**
 * TruthToText consumes a boolean or numeric value, and outputs the
 * specific truth strings.  This is the inverse of TextToTruth.
 */
template <class IN>
class TruthToText : public Transform<IN, String> {

  public:
    TruthToText(const char* trueValue="ON", const char* falseValue="OFF") {
        truthValue = new char*[2];
        truthValue[0] = falseValue;
        truthValue[1] = trueValue;
    }

    virtual void set_input(IN input, uint8_t input_channel = 0) override {
       if (input) {
          this->emit(truthValue[1]);
       }
       else {
          this->emit(truthValue[0]);
       }
    }

  protected:
     char* truthValue[];

};


#endif