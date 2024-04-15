#include "curveinterpolator.h"

#include "sensesp.h"

namespace sensesp {

CurveInterpolator::Sample::Sample() {}

CurveInterpolator::Sample::Sample(float input, float output)
    : input{input}, output{output} {}

CurveInterpolator::Sample::Sample(JsonObject& obj)
    : input{obj["input"]}, output{obj["output"]} {}

/**
 * @brief Construct a new CurveInterpolator object
 *
 * @param defaults Default sample values
 * @param config_path
 * @param input_title Display name for the input sample values
 * @param output_title Display name for the output sample values
 */
CurveInterpolator::CurveInterpolator(std::set<Sample>* defaults,
                                     String config_path)
    : FloatTransform(config_path) {
  // Load default values if no configuration present...
  if (defaults != NULL) {
    samples_.clear();
    samples_ = *defaults;
  }

  load_configuration();
}

void CurveInterpolator::set_input(float input, uint8_t inputChannel) {
    if (samples_.empty()) {
        output = 0;  // or any default output if no samples are available
        notify();
        return;
    }

    std::set<Sample>::iterator it = samples_.begin();
    float x0 = it->input;
    float y0 = it->output;

    // Check if the input is below the lowest sample point
    if (input < x0) {
        // Need to extrapolate below the first point
        if (samples_.size() > 1) {
            auto second_it = std::next(it);
            float x1 = second_it->input;
            float y1 = second_it->output;
            float gradient = (y1 - y0) / (x1 - x0);

            output = y0 + gradient * (input - x0);  // Extrapolate using the first segment's gradient
        } else {
            output = y0;  // Only one sample, output its value
        }
        notify();
        return;
    }

    // Search for the correct interval or the last sample point
    while (it != samples_.end()) {
        if (input > it->input) {
            x0 = it->input;
            y0 = it->output;
            ++it;
        } else {
            break;
        }
    }

    // Interpolate or extrapolate above the highest point
    if (it != samples_.end()) {
        float x1 = it->input;
        float y1 = it->output;
        output = (y0 * (x1 - input) + y1 * (input - x0)) / (x1 - x0);
    } else {
        // Hit the end of the table with no match, calculate output using the gradient from the last two points
        auto last = samples_.rbegin();
        auto second_last = std::next(last);
        float x1 = last->input;
        float y1 = last->output;
        float x2 = second_last->input;
        float y2 = second_last->output;
        float gradient = (y1 - y2) / (x1 - x2);

        // Extrapolate using the gradient
        output = y1 + gradient * (input - x1);
    }

    notify();
}



void CurveInterpolator::get_configuration(JsonObject& root) {
  JsonArray json_samples = root.createNestedArray("samples");
  for (auto& sample : samples_) {
    auto entry = json_samples.createNestedObject();
    if( entry.isNull() ) {
      debugE("No memory for sample");
    }
    entry["input"] = sample.input;
    entry["output"] = sample.output;
  }
}

static const char SCHEMA[] PROGMEM = R"({
    "type": "object",
    "properties": {
        "samples": { "title": "Sample values",
                    "type": "array",
                    "format": "table",
                    "items": { "type": "object",
                               "properties": {
                                   "input": { "type": "number",
                                              "title": "%s" },
                                   "output": { "type": "number",
                                              "title": "%s" }
                             }}}
    }
  })";

String CurveInterpolator::get_config_schema() {
  char buf[sizeof(SCHEMA) + 160];
  snprintf_P(buf, sizeof(buf), SCHEMA, input_title_.c_str(),
             output_title_.c_str());
  return String(buf);
}

bool CurveInterpolator::set_configuration(const JsonObject& config) {
  String expected[] = {"samples"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      debugE(
          "Can not set CurveInterpolator configuration: missing json field "
          "%s\n",
          str.c_str());
      return false;
    }
  }

  JsonArray arr = config["samples"];
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
  Sample* sample_copy = new Sample(sample);
  samples_.insert(*sample_copy);
}

}  // namespace sensesp
