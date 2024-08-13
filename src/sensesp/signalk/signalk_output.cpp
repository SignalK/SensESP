#include "signalk_output.h"

namespace sensesp {

template <typename T>
SKOutputNumeric<T>::SKOutputNumeric(String sk_path, String config_path,
                                    SKMetadata* meta)
    : SKOutput<T>(sk_path, config_path, meta) {
  if (this->meta_ == NULL && !this->sk_path.isEmpty()) {
    ESP_LOGW(
        __FILENAME__,
        "WARNING - No metadata for %s. Numeric values should specify units",
        this->sk_path.c_str());
  }
}

template class SKOutputNumeric<float>;
template class SKOutputNumeric<int>;

}  // namespace sensesp
