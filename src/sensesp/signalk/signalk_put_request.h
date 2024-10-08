#ifndef SENSESP_SIGNALK_SIGNALK_PUT_REQUEST_H_
#define SENSESP_SIGNALK_SIGNALK_PUT_REQUEST_H_

#include <ArduinoJson.h>
#include <functional>
#include <map>

#include "sensesp/ui/config_item.h"
#include "sensesp/system/valueconsumer.h"

namespace sensesp {

/**
 * @brief A base class for all objects that are capable of sending
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
   * @return The request Id that is sent to the server.
   */
  static String send_request(JsonDocument& request,
                             std::function<void(JsonDocument&)> callback,
                             uint32_t timeout = 5000);

  /**
   * Is called by the web socket code to handle any incoming request responses
   * that is receives.
   */
  static void handle_response(JsonDocument& response);

 protected:
  /// PendingRequest is a utility class used internally for
  /// tracking websocket requests made to the SignalK server.
  class PendingRequest {
   public:
    String request_id;
    std::function<void(JsonDocument&)> callback;
    reactesp::DelayEvent* timeout_cleanup;
  };

  /// A map to hold all of the requests we are expecting future
  /// responses from...
  static std::map<String, PendingRequest*> request_map_;

  /// Removes the specified request_id from the request_map,
  /// cleaning up outstanding events if necessary
  static void remove_request(String request_id);

  /// Returns the pending request object associated with
  /// request_id, or nullptr if no such id is found.
  static PendingRequest* get_request(String request_id);
};

/**
 * @brief  A base class for all template variations of
 * the PUT request class SKPutRequest. This base class keeps the
 * compiler from generating reduntant object code for common
 * functionality of each template version.
 * @see SKPutRequest
 * See https://signalk.org/specification/1.5.0/doc/put.html
 */
class SKPutRequestBase : public SKRequest,
                         public FileSystemSaveable {
 public:
  /**
   * The constructor
   * @param sk_path The SignalK path the put request will be made on
   * @param config_path The configuration path to save the configuration
   * @param timeout The number of milliseconds to wait for a COMPLETED or
   *  FAILED response to be received from the server
   */
  SKPutRequestBase(const String& sk_path, const String& config_path = "",
                   uint32_t timeout = 5000);

  // For reading and writing the configuration
  virtual bool to_json(JsonObject& root) override;
  virtual bool from_json(const JsonObject& config) override;

  /**
   * Returns the Signal K path this object makes requests to
   */
  String get_sk_path() { return sk_path; }

  /**
   * Returns TRUE if there is currently a PUT request pending
   * (i.e. this class has send a request, and it has not yet
   * received a reply or timeout)
   */
  bool request_pending();

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
  virtual void on_response(JsonDocument& response);

  String sk_path{};
  uint32_t timeout{};
  String pending_request_id_{};
};

/**
 * @brief Used to send requests to the server to change the
 * value of the specified path to a specific value according
 * to the specification at
 * https://signalk.org/specification/1.5.0/doc/put.html
 */
template <typename T>
class SKPutRequest : public SKPutRequestBase,
                     public ValueConsumer<T> {
 public:
  /**
   * The constructor
   * @param sk_path The SignalK path the put request will be made on
   * @param config_path The configuration path to save the configuration
   * @param ignore_duplicates TRUE if incoming values that have not changed
   *  since the last one should not generated a PUT request
   * @param timeout The number of milliseconds to wait for a COMPLETED or
   *  FAILED response to be received from the server
   */
  SKPutRequest(String sk_path, String config_path = "",
               bool ignore_duplicates = true, uint32_t timeout = 5000)
      : SKPutRequestBase(sk_path, config_path, timeout),
        ignore_duplicates{ignore_duplicates} {}

  virtual void set(const T& new_value) override {
    if (ignore_duplicates && new_value == value) {
      return;
    }
    if (!request_pending()) {
      this->value = new_value;
      send_put_request();
    } else {
      ESP_LOGW(__FILENAME__,
               "Ignoring PUT request (previous request still outstanding)");
    }
  };

  virtual void set_put_value(JsonObject& put_data) override {
    put_data["value"] = value;
  };

 protected:
  T value;
  bool ignore_duplicates;
};

template <typename T>
const String ConfigSchema(const SKPutRequest<T>& obj) {
  static const char schema[] = R"###({"type":"object","properties":{"sk_path":{"title":"Signal K Path","type":"string"}}  })###";
  return schema;
}

typedef SKPutRequest<float> FloatSKPutRequest;
typedef SKPutRequest<int> IntSKPutRequest;
typedef SKPutRequest<bool> BoolSKPutRequest;
typedef SKPutRequest<String> StringSKPutRequest;

}  // namespace sensesp

#endif
