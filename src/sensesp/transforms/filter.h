#ifndef SENSESP_SRC_TRANSFORMS_FILTER_H_
#define SENSESP_SRC_TRANSFORMS_FILTER_H_

#include <functional>

#include "sensesp/transforms/transform.h"

namespace sensesp {

/**
 * @brief Transform that only emits the output if the filter condition returns
 * true.
 *
 */
template <typename T>
class Filter : public Transform<T, T> {
 public:
  Filter(std::function<bool(const T&)> filter, String config_path = "")
      : Transform<T, T>(config_path), filter_{filter} {
    this->load();
  }
  virtual void set(const T& new_value) override {
    if (filter_(new_value)) {
      this->emit(new_value);
    }
  }

 private:
  std::function<bool(const T&)> filter_;
};

}  // namespace sensesp

#endif  // SENSESP_SRC_TRANSFORMS_FILTER_H_
