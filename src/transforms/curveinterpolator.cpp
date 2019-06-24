#include "curveinterpolator.h"

#include <RemoteDebug.h>

CurveInterpolator::Sample::Sample() {
}

CurveInterpolator::Sample::Sample(float input, float output) : input{input}, output{output} {
}

CurveInterpolator::Sample::Sample(JsonObject& obj) : input{obj["input"]}, output{obj["output"]} {

}


CurveInterpolator::CurveInterpolator(std::set<Sample>* defaults, String config_path)
    : NumericTransform(config_path) {

  className = "CurveInterpolator";    

  // Load default values if no configuration present...
  if (defaults != NULL) {
    samples.clear();
    samples = *defaults;
  }

  load_configuration();

}


void CurveInterpolator::set_input(float input, uint8_t inputChannel) {

  float x0 = 0.0;
  float y0 = 0.0;

  std::set<Sample>::iterator it = samples.begin();

  while (it != samples.end()) {
     auto& sample = *it;

     if (input > sample.input) {
        x0 = sample.input;
        y0 = sample.output;
     }
     else {
       break;
     }

     it++;
  }

  if (it != samples.end()) {
    // We found our range. "input" is between
    // minInput and it->input. CurveInterpolator the result
    Sample max = *it;
    float x1 = max.input;
    float y1 = max.output;

    output = (y0 * (x1 - input) + y1 * (input - x0)) / (x1 - x0);
  }
  else {
    // Hit the end of the table with no match.
    output = 9999.9;
//    debugW("Could not find sample interval for input %0f", input);
  }

  notify();
}


JsonObject& CurveInterpolator::get_configuration(JsonBuffer& buf) {
  JsonObject& root = buf.createObject();

  JsonArray& jSamples = root.createNestedArray("samples");
  for (auto& sample : samples) {
    JsonObject& entry = buf.createObject();
    entry["input"] = sample.input;
    entry["output"] = sample.output;
    jSamples.add(entry);
  }
  return root;
}

static const char SCHEMA[] PROGMEM = R"({
    "type": "object",
    "properties": {
        "samples": { "title": "Sample values",
                    "type": "array",
                    "items": { "title": "Sample",
                                "type": "object",
                                "properties": {
                                    "input": { "type": "number" },
                                    "output": { "type": "number" }
                              }}}
    }
  })";

String CurveInterpolator::get_config_schema() {
  return FPSTR(SCHEMA);
}

bool CurveInterpolator::set_configuration(const JsonObject& config) {

  String expected[] = { "samples" };
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      debugE("Can not set CurveInterpolator configuration: missing json field %s\n", str.c_str());
      return false;
    }
  }

  JsonArray& arr = config["samples"];
  if (arr.size() > 0) {
    samples.clear();
    for (auto& jentry : arr) {
        Sample sample(jentry.as<JsonObject>());
        samples.insert(sample);
    }

  }

  return true;

}

void CurveInterpolator::clearSamples() {
   samples.clear();
}

void CurveInterpolator::addSample(const Sample& sample) {
   Sample* pSampleCopy = new Sample(sample);
   samples.insert(*pSampleCopy);
}
