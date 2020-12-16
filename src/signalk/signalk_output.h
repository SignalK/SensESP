#ifndef _signalk_output_H_
#define _signalk_output_H_

#include "signalk/signalk_emitter.h"
#include "transforms/transform.h"

static const char SIGNALKOUTPUT_SCHEMA[] PROGMEM = R"({
      "type": "object",
      "properties": {
          "sk_path": { "title": "Signal K Path", "type": "string" }
      }
  })";

// @brief A specialized transform whose primary purpose is
// to output Signal K data on the Signal K network.
template <typename T>
class SKOutput : public SKEmitter, public SymmetricTransform<T> {
 public:
  SKOutput() : SKOutput("") { this->load_configuration(); }

  /**
   * The constructor
   * @param sk_path The Signal K path the output value of this transform is sent on
   * @param config_path The optional configuration path that allows an end user to
   *   change the configuration of this object. See the Configurable class for more information.
   * @param meta Optional metadata that is associated with the value output by this class
   *   A value specified here will cause the path's metadata to be emitted on the first
   *   delta sent to the server. Use NULL if this path has no metadata to report (or
   *   if the path is already an official part of the Signal K specification)
   */
  SKOutput(String sk_path, String config_path = "", SKMetadata* meta = NULL)
      : SKEmitter(sk_path), SymmetricTransform<T>(config_path), meta_{meta} {
    Enable::set_priority(-5);
    this->load_configuration();
  }


  SKOutput(String sk_path, SKMetadata* meta) :
    SKOutput(sk_path, "", meta) {}


  virtual void set_input(T new_value, uint8_t input_channel = 0) override {
    this->ValueProducer<T>::emit(new_value);
  }

  virtual String as_signalk() override {
    DynamicJsonDocument json_doc(1024);
    String json;
    json_doc["path"] = this->get_sk_path();
    json_doc["value"] = ValueProducer<T>::output;
    serializeJson(json_doc, json);
    return json;
  }

  virtual void get_configuration(JsonObject& root) override {
    root["sk_path"] = this->get_sk_path();
  }

  String get_config_schema() override { return FPSTR(SIGNALKOUTPUT_SCHEMA); }

  virtual bool set_configuration(const JsonObject& config) override {
    if (!config.containsKey("sk_path")) {
      return false;
    }
    this->set_sk_path(config["sk_path"].as<String>());
    return true;
  }

  /**
   * Used to set the optional metadata that is associated with
   * the Signal K path this transform emits. This is a second
   * method of setting the metadata (the first being a parameter
   * to the constructor).
   */
  virtual void set_metadata(SKMetadata* meta) { this->meta_ = meta; }


  virtual SKMetadata* get_metadata() override { return meta_; }

  protected:
    SKMetadata* meta_;
};


/**
 * @brief A special class for sending numeric values to
 * the Signal K server on a specific Signal K path.
 */
template <typename T>
class SKOutputNumeric : public SKOutput<T> {

   public:
      SKOutputNumeric(String sk_path, String config_path = "", SKMetadata* meta = NULL);


      SKOutputNumeric(String sk_path, SKMetadata* meta) :
        SKOutputNumeric(sk_path, "", meta) {}


      /// The Signal K specification requires that numeric values sent
      /// to the server should at minimum specify a "units". This
      /// constructor allows you to conveniently specify the numeric
      /// units as a third parameter.
      /// @param units The unit value for the number this outputs on the specified
      ///  Signal K path. See https://github.com/SignalK/specification/blob/master/schemas/definitions.json#L87
      ///   
      /// @see SKMetadata
      SKOutputNumeric(String sk_path, String config_path, String units) :
         SKOutputNumeric(sk_path, config_path, new SKMetadata(units)) {}
};

typedef SKOutputNumeric<float> SKOutputNumber;
typedef SKOutputNumeric<int> SKOutputInt;
typedef SKOutput<bool> SKOutputBool;
typedef SKOutput<String> SKOutputString;

#endif
