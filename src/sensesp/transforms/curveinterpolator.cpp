#include "sensesp.h"

#include "curveinterpolator.h"

namespace sensesp {

CurveInterpolator::Sample::Sample() = default;

CurveInterpolator::Sample::Sample(float input, float output)
    : input_{input}, output_{output} {}

CurveInterpolator::Sample::Sample(JsonObject& obj)
    : input_{obj["input"]}, output_{obj["output"]} {}

/**
 * @brief Construct a new CurveInterpolator object
 *
 * @param defaults Default sample values
 * @param config_path
 * @param input_title Display name for the input sample values
 * @param output_title Display name for the output sample values
 */
CurveInterpolator::CurveInterpolator(std::set<Sample>* defaults,
                                     const String& config_path)
    : FloatTransform(config_path) {
  // Load default values if no configuration present...
  if (defaults != NULL) {
    samples_.clear();
    samples_ = *defaults;
  }

  load();
}

void CurveInterpolator::set(const float& input) {
  if (samples_.empty()) {
    output_ = 0;  // or any default output if no samples are available
    notify();
    return;
  }

  std::set<Sample>::iterator it = samples_.begin();
  float x0 = it->input_;
  float y0 = it->output_;

  // Check if the input is below the lowest sample point
  if (input < x0) {
    // Need to extrapolate below the first point
    if (samples_.size() > 1) {
      auto second_it = std::next(it);
      float x1 = second_it->input_;
      float y1 = second_it->output_;
      float const gradient = (y1 - y0) / (x1 - x0);

      output_ = y0 + gradient *
                        (input -
                         x0);  // Extrapolate using the first segment's gradient
    } else {
      output_ = y0;  // Only one sample, output its value
    }
    notify();
    return;
  }

  // Search for the correct interval or the last sample point
  while (it != samples_.end()) {
    if (input > it->input_) {
      x0 = it->input_;
      y0 = it->output_;
      ++it;
    } else {
      break;
    }
  }

  // Interpolate or extrapolate above the highest point
  if (it != samples_.end()) {
    float x1 = it->input_;
    float y1 = it->output_;
    output_ = (y0 * (x1 - input) + y1 * (input - x0)) / (x1 - x0);
  } else {
    // Hit the end of the table with no match, calculate output using the
    // gradient from the last two points
    auto last = samples_.rbegin();
    auto second_last = std::next(last);
    float x1 = last->input_;
    float y1 = last->output_;
    float x2 = second_last->input_;
    float y2 = second_last->output_;
    float const gradient = (y1 - y2) / (x1 - x2);

    // Extrapolate using the gradient
    output_ = y1 + gradient * (input - x1);
  }

  notify();
}

bool CurveInterpolator::to_json(JsonObject& doc) {
  JsonArray json_samples = doc["samples"].to<JsonArray>();
  for (auto& sample : samples_) {
    // Add a new JsonObject to the array
    JsonObject entry = json_samples.add<JsonObject>();
    if (entry.isNull()) {
      ESP_LOGE(__FILENAME__, "No memory for sample");
      return false;
    }
    entry["input"] = sample.input_;
    entry["output"] = sample.output_;
  }
  return true;
}

bool CurveInterpolator::from_json(const JsonObject& doc) {
  String const expected[] = {"samples"};
  for (auto str : expected) {
    if (!doc[str].is<JsonVariant>()) {
      ESP_LOGE(
          __FILENAME__,
          "Can not set CurveInterpolator configuration: missing json field "
          "%s\n",
          str.c_str());
      return false;
    }
  }

  JsonArray arr = doc["samples"];
  if (arr.size() > 0) {
    samples_.clear();
    for (auto jentry : arr) {
      auto json_object = jentry.as<JsonObject>();
      Sample sample(json_object);
      samples_.insert(sample);
    }
  }

  return true;
}

void CurveInterpolator::clear_samples() { samples_.clear(); }

void CurveInterpolator::add_sample(const Sample& sample) {
  auto* sample_copy = new Sample(sample);
  samples_.insert(*sample_copy);
}

}  // namespace sensesp
