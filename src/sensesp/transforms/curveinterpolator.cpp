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
  float x0 = 0.0;
  float y0 = 0.0;

  std::set<Sample>::iterator it = samples_.begin();

  while (it != samples_.end()) {
    auto& sample = *it;

    if (input > sample.input) {
      x0 = sample.input;
      y0 = sample.output;
    } else {
      break;
    }

    it++;
  }

  if (it != samples_.end()) {
    // We found our range. "input" is between
    // minInput and it->input. CurveInterpolator the result
    Sample max = *it;
    float x1 = max.input;
    float y1 = max.output;

    output = (y0 * (x1 - input) + y1 * (input - x0)) / (x1 - x0);
  } else {
    // Hit the end of the table with no match.
    output = 9999.9;
    //    debugW("Could not find sample interval for input %0f", input);
  }

  notify();
}

void CurveInterpolator::get_configuration(JsonObject& root) {
  JsonArray json_samples = root["samples"].to<JsonArray>();
  for (auto& sample : samples_) {
    auto entry = json_samples.add<JsonObject>();
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
