#include "truth_text.h"

bool textIsTrue(String input) {

  if (input.length() == 0) {
      return false;
  }

  if (input.equalsIgnoreCase("On") ||
      input.equalsIgnoreCase("Yes") ||
      input.equalsIgnoreCase("True") ||
      input.equalsIgnoreCase("T") ||
      input.equalsIgnoreCase("Y")
     ) {
    return true;
  }

  if (input.toInt() != 0) {
      return true;
  }

  return false;

}


bool textIsFalse(String input) {

  if (input.length() == 0) {
      return false;
  }

  if (input.equalsIgnoreCase("Off") ||
      input.equalsIgnoreCase("No") ||
      input.equalsIgnoreCase("False") ||
      input.equalsIgnoreCase("F") ||
      input.equalsIgnoreCase("N") ||
      input.equalsIgnoreCase("0") ||
      input.equalsIgnoreCase("0.0")      
     ) {
    return true;
  }

  return false;

}


template<class OUT>
void TextToTruth<OUT>::set_input(String input, uint8_t input_channel) {
     this->emit(textIsTrue(input));
}

template<>
void TextToTruth<int>::set_input(String input, uint8_t input_channel) {
    this->emit(textIsTrue(input) ? 1 : 0);
}

template<>
void TextToTruth<float>::set_input(String input, uint8_t input_channel) {
    this->emit(textIsTrue(input) ? 1.0 : 0.0);
}
