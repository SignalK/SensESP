#include "truth_text.h"

#include <utility>

namespace sensesp {

bool TextToTruth::is_valid_true(const String& value) {
  if (value.length() == 0) {
    return false;
  }

  if (value.equalsIgnoreCase("On") || value.equalsIgnoreCase("Yes") ||
      value.equalsIgnoreCase("True") || value.equalsIgnoreCase("T") ||
      value.equalsIgnoreCase("Y")) {
    return true;
  }

  if (value.toInt() != 0) {
    return true;
  }

  return false;
}

bool TextToTruth::is_valid_false(const String& value) {
  if (value.length() == 0) {
    return false;
  }

  if (value.equalsIgnoreCase("Off") || value.equalsIgnoreCase("No") ||
      value.equalsIgnoreCase("False") || value.equalsIgnoreCase("F") ||
      value.equalsIgnoreCase("N") || value.equalsIgnoreCase("0") ||
      value.equalsIgnoreCase("0.0")) {
    return true;
  }

  return false;
}

void TextToTruth::set(const String& input) {
  this->emit(TextToTruth::is_valid_true(input));
}

TruthToText::TruthToText(const String& true_value, const String& false_value)
    : Transform<bool, String>(), truth_value_{false_value, true_value} {}

void TruthToText::set(const bool& input) {
  if (input) {
    this->emit(truth_value_[1]);
  } else {
    this->emit(truth_value_[0]);
  }
}

}  // namespace sensesp
