#ifndef SENSESP_TRANSFORMS_TRUTH_TEXT_H_
#define SENSESP_TRANSFORMS_TRUTH_TEXT_H_

#include "transform.h"

namespace sensesp {

/**
 * @brief Consumes a string and outputs boolean "true"
 * if the text represents a human readable version of "true".
 *
 * The following
 * case-insensitive values are considered true:
 * <p><ul>
 *   <li>True
 *   <li>T
 *   <li>Yes
 *   <li>Y
 *   <li>On
 *   <li>Any string that represents a non-zero number
 * </ul>
 * @see TruthToText
 */
class TextToTruth : public Transform<String, bool> {
 public:
  virtual void set(const String& input) override;

  /**
   * Returns TRUE if `value` represents one of the truth values recognized by
   * TextToTruth
   */
  static bool is_valid_true(const String& value);

  /**
   * Returns TRUE if `value` represents one of the boolean values recognized
   * as a "false" value. FALSE is returned if `value` is a value that is
   * normally associated with a boolean false.
   */
  static bool is_valid_false(const String& value);
};

/**
 * @brief Consumes a boolean value and outputs one the
 * appropriate truth string specified in the constructor.
 * This is the inverse of TextToTruth.
 * @see TextToTruth
 */
class TruthToText : public Transform<bool, String> {
 public:
  TruthToText(const String& true_value = "ON",
              const String& false_value = "OFF");

  virtual void set(const bool& input) override;

 protected:
  String truth_value_[2];
};

}  // namespace sensesp
#endif
