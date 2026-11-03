#ifndef SENSESP_SRC_SENSESP_SIGNALK_SIGNALK_DELTA_PACKING_H_
#define SENSESP_SRC_SENSESP_SIGNALK_SIGNALK_DELTA_PACKING_H_

#include <cstddef>

namespace sensesp {

/**
 * @brief Tracks how long the delta being built is, so a batch can be split
 * into deltas that each stay within the transport's budget.
 *
 * The queued values are already serialized, and the envelope around them is
 * fixed for a given delta, so the length of the delta is known without
 * serializing it: the envelope, plus each item, plus one separating comma
 * between consecutive items. The caller asks fits() before each item and
 * starts a new delta when the answer is no.
 *
 * The budget boundary matches sk_delta_exceeds_ws_buffer(): a delta of exactly
 * the budget is sent as one chunk, so exactly the budget fits. A budget of 0
 * means no limit.
 *
 * An item longer than the budget fits nowhere, and fits() keeps saying no.
 * The caller must add it to an empty delta anyway rather than loop; that delta
 * then carries one item and the transport decides what to do with it.
 *
 * Pure and dependency-free (no Arduino, no ArduinoJson) so it can be
 * unit-tested on the host.
 */
class SKDeltaPacker {
 public:
  SKDeltaPacker(size_t envelope_length, size_t budget)
      : envelope_length_{envelope_length},
        budget_{budget},
        length_{envelope_length},
        item_count_{0} {}

  /// True if adding an item of this length would keep the delta within budget.
  bool fits(size_t item_length) const {
    if (budget_ == 0) {
      return true;
    }
    return length_ + separator_length() + item_length <= budget_;
  }

  void add(size_t item_length) {
    length_ += separator_length() + item_length;
    item_count_++;
  }

  /// Start the next delta: the envelope again, with no items.
  void reset() {
    length_ = envelope_length_;
    item_count_ = 0;
  }

  bool is_empty() const { return item_count_ == 0; }
  size_t item_count() const { return item_count_; }
  size_t length() const { return length_; }

 private:
  size_t separator_length() const { return item_count_ == 0 ? 0 : 1; }

  size_t envelope_length_;
  size_t budget_;
  size_t length_;
  size_t item_count_;
};

}  // namespace sensesp

#endif  // SENSESP_SRC_SENSESP_SIGNALK_SIGNALK_DELTA_PACKING_H_
