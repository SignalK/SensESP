#include "interpolate.h"

#include <RemoteDebug.h>

Interpolate::Sample::Sample() {
}

Interpolate::Sample::Sample(float input, float output) : input{input}, output{output} {
}

Interpolate::Sample::Sample(JsonObject& obj) : input{obj["input"]}, output{obj["output"]} {
  
}


Interpolate::Interpolate(String sk_path, std::set<Sample>* defaults, String config_path)
    : OneToOneTransform<float>{ sk_path, config_path } {

  // Load default values if no configuration present...
  if (defaults != NULL) {
    samples.clear();
    samples = *defaults;
  }

  load_configuration();

}


void Interpolate::set_input(float input, uint8_t inputChannel) {

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
    // minInput and it->input. Interpolate the result
    Sample max = *it;
    float x1 = max.input;
    float y1 = max.output;

    output = (y0 * (x1 - input) + y1 * (input - x0)) / (x1 - x0);
  }
  else {
    // Hit the end of the table with no match.
    output = 9999.9;
    debugE("Could not find sample interval for input %0f", input);
  }

  notify();
}


String Interpolate::as_signalK() {
  DynamicJsonBuffer jsonBuffer;
  String json;
  JsonObject& root = jsonBuffer.createObject();
  root.set("path", this->sk_path);
  root.set("value", output);
  root.printTo(json);
  return json;
}


JsonObject& Interpolate::get_configuration(JsonBuffer& buf) {
  JsonObject& root = buf.createObject();
  root["sk_path"] = sk_path;

  JsonArray& jSamples = root.createNestedArray("samples");
  for (auto& sample : samples) {
    JsonObject& entry = buf.createObject();
    entry["input"] = sample.input;
    entry["output"] = sample.output;
    jSamples.add(entry);
  }
  return root;
}


bool Interpolate::set_configuration(const JsonObject& config) {

  String expected[] = { "sk_path", "samples" };
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      debugE("Can not set Interpolate configuration: missing json field %s\n", str.c_str());
      return false;
    }
  }

  sk_path = config["sk_path"].as<String>();

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

void Interpolate::clearSamples() {
   samples.clear();
}

void Interpolate::addSample(const Sample& sample) {
   Sample* pSampleCopy = new Sample(sample);
   samples.insert(*pSampleCopy);
}
