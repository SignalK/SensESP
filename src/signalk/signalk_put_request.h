#ifndef _signalk_put_request_H_
#define _signalk_put_request_H_

#include <ArduinoJson.h>

#include <functional>
#include <map>

#include "Arduino.h"
#include "ReactESP.h"
#include "system/configurable.h"
#include "system/valueconsumer.h"

/**
 * SKRequest is a base class for all objects that are capable of sending
 * "requests" to the SignalK server (and optionally receiving the responses)
 *  according to the specification located at
 * https://signalk.org/specification/1.5.0/doc/request_response.html
 */
class SKRequest {
 public:
  /**
   * Sends the specified request to the server via the connected websocket.
   * @param request The json document to send for the request. This document
   * should follow the formats specified in the SignalK Request/Response
   * specification with the exception that it should not contain a
   * "requestId" (as this method will add it).
   * @param callback The callback method that processes any and all responses
   * from the server that correspond to the request.
   * @param timeout The number of milliseconds to wait for a COMPLETED or
   *  FAILED response to be received from the server
   */
  static void send_request(DynamicJsonDocument& request,
                           std::function<void(DynamicJsonDocument&)> callback,
                           uint32_t timeout = 5000);

  /**
   * Is called by the web socket code to handle any incoming request responses
   * that is receives.
   */
  static void handle_response(DynamicJsonDocument& response);

 private:
  // PendingRequest is a utility class used internally for
  // tracking websocket requests made to the SignalK server.
  class PendingRequest {
   public:
    String request_id;
    std::function<void(DynamicJsonDocument&)> callback;
    DelayReaction* timeout_cleanup;
  };

  // A map to hold all of the requests we are expecting future
  // responses from...
  static std::map<String, PendingRequest*> request_map;

  // Removes the specified request_id from the request_map,
  // cleaning up outstanding reactions if necessary
  static void remove_request(String request_id);

  // Returns the pending request object associated with
  // request_id, or nullptr is no such id is found.
  static PendingRequest* get_request(String request_id);
};

/**
 * SKPutRequestBase is a base class for all variations of
 * put requests
 */
class SKPutRequestBase : public SKRequest, public Configurable {
 public:
  SKPutRequestBase(String sk_path, String config_path = "");

  // For reading and writing the configuration
  virtual void get_configuration(JsonObject& doc) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;

 protected:
  /**
   * Sends the put request to the server
   */
  void send_put_request();

  /**
   * Sets the "value" field of the put request with the
   * appropriate value.
   */
  virtual void set_put_value(JsonObject& put_data) = 0;

  /**
   * Called whenever a response to a request has been
   * received from the server
   */
  virtual void on_response(DynamicJsonDocument& response);

  String sk_path;
};

template <typename T>
class SKPutRequest : public SKPutRequestBase, public ValueConsumer<T> {
 public:
  SKPutRequest(String sk_path, String config_path = "")
      : SKPutRequestBase(sk_path, config_path) {}

  virtual void set_input(T new_value, uint8_t input_channel = 0) override {
    this->value = new_value;
    send_put_request();
  };

  virtual void set_put_value(JsonObject& put_data) override {
    put_data["value"] = value;
  };

 protected:
  T value;
};

typedef SKPutRequest<float> SKNumericPutRequest;
typedef SKPutRequest<int> SKIntegerPutRequest;
typedef SKPutRequest<bool> SKBooleanPutRequest;
typedef SKPutRequest<String> SKStringPutRequest;

#endif