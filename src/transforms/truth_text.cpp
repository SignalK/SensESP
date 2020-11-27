#include "truth_text.h"

bool is_valid_true(String input) {

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


bool is_valid_false(String input) {

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


void TextToTruth::set_input(String input, uint8_t input_channel) {
     this->emit(is_valid_true(input));
}


TruthToText::TruthToText(String true_value, String false_value) :
       Transform<bool, String>() {
    truth_value = new String[2];
    truth_value[0] = false_value;
    truth_value[1] = true_value;
}

void TruthToText::set_input(bool input, uint8_t input_channel) {
   if (input) {
      this->emit(truth_value[1]);
   }
   else {
      this->emit(truth_value[0]);
   }
}
