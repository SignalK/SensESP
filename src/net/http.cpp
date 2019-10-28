#include "http.h"

#include <functional>

#include <FS.h>
#ifdef ESP32
#include "SPIFFS.h"
#endif

#include "Arduino.h"
#include "AsyncJson.h"
#include "ArduinoJson.h"
#include <ESPAsyncWebServer.h>

#include "sensesp_app.h"
#include "system/configurable.h"

// Include the web UI stored in PROGMEM space
#include "web/index.h"
#include "web/setup.h"
#include "web/js_jsoneditor.h"
#include "web/js_sensesp.h"

// HTTP port for the configuration interface
#ifndef HTTP_SERVER_PORT
#define HTTP_SERVER_PORT 80
#endif

HTTPServer::HTTPServer(std::function<void()> reset_device) {
  this->reset_device = reset_device;
  server = new AsyncWebServer(HTTP_SERVER_PORT);
  using std::placeholders::_1;

  server->onNotFound(std::bind(&HTTPServer::handle_not_found, this, _1));

  // Handle setting configuration values of a Configurable via a Json PUT to /config
  AsyncCallbackJsonWebHandler* config_put_handler
    = new AsyncCallbackJsonWebHandler(
      "/config",
      [](AsyncWebServerRequest *request, JsonVariant &json) {
        // omit the "/config" part of the url
        String url_tail = request->url().substring(7);

        if (url_tail=="") {
          request->send(405, "text/plain",
                        F("PUT to /config not allowed.\n"));
          return;
        }

        std::map<String, Configurable*>::iterator it
            = configurables.find(url_tail);
        if (it==configurables.end()) {
          request->send(404, "text/plain",
                        F("Configuration key not found.\n"));
          return;
        }
        Configurable* confable = it->second;

        JsonObject& body = json.as<JsonObject>();
        if (body.success()) {
          if (!confable->set_configuration(body)) {
            request->send(400, "text/plain",
                          F("Unable to extract keys from JSON.\n"));
            return;
          }
          confable->save_configuration();
          request->send(200, "text/plain", F("Configuration successful.\n"));
          return;
        } else {
          request->send(400, "text/plain", F("Unable to parse JSON body.\n"));
          return;
        }
      });
  config_put_handler->setMethod(HTTP_PUT);
  server->addHandler(config_put_handler);


  // Handle requests to retrieve the current Json configuration of a Configurable
  // via HTTP GET on /config
  server->on("/config", HTTP_GET, [this] (AsyncWebServerRequest *request) {
    // omit the "/config" part of the url
    String url_tail = request->url().substring(7);

    if (url_tail=="" || url_tail=="/") {
      this->handle_config_list(request);
      return;
    }

    std::map<String, Configurable*>::iterator it
        = configurables.find(url_tail);
    if (it==configurables.end()) {
      request->send(404, "text/plain",
                    F("Configuration key not found.\n"));
      return;
    }
    Configurable* confable = it->second;

    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonBuffer json_buffer;
    JsonObject& root = json_buffer.createObject();
    root["config"] = confable->get_configuration(json_buffer);
    root["schema"] = RawJson(confable->get_config_schema());
    root.printTo(*response);
    request->send(response);
  });


  server->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
      debugD("Serving index.html");
      request->send_P(200, "text/html", PAGE_index);
  });

  server->on("/setup", HTTP_GET, [](AsyncWebServerRequest *request) {
      debugD("Serving setup.html");
      request->send_P(200, "text/html", PAGE_setup);
  });

  server->on("/js/jsoneditor.min.js", HTTP_GET, [](AsyncWebServerRequest *request) {
      debugD("Serving jsoneditor.min.js");
      request->send_P(200, "text/javascript", PAGE_js_jsoneditor);
  });


  server->on("/js/sensesp.js", HTTP_GET, [](AsyncWebServerRequest *request) {
      debugD("Serving sensesp.js");
      request->send_P(200, "text/javascript", PAGE_js_sensesp);
  });


  server->on("/device/reset", HTTP_GET,
             std::bind(&HTTPServer::handle_device_reset, this, _1));
  server->on("/device/restart", HTTP_GET,
             std::bind(&HTTPServer::handle_device_restart, this, _1));
  server->on("/info", HTTP_GET,
             std::bind(&HTTPServer::handle_info, this, _1));
}


void HTTPServer::handle_not_found(AsyncWebServerRequest* request) {
  debugD("NOT_FOUND: ");
  if(request->method() == HTTP_GET) {
     debugD("GET"); 
  }
  else if(request->method() == HTTP_POST) {
    debugD("POST");
  }
  else if(request->method() == HTTP_DELETE) {
    debugD("DELETE");
  }
  else if(request->method() == HTTP_PUT) {
    debugD("PUT");
  }
  else if(request->method() == HTTP_PATCH) {
    debugD("PATCH");
  }
  else if(request->method() == HTTP_HEAD) {
    debugD("HEAD");
  }
  else if(request->method() == HTTP_OPTIONS) {
    debugD("OPTIONS");
  }
  else {
    debugD("UNKNOWN");
  }
  debugD(" http://%s%s\n", request->host().c_str(), request->url().c_str());

  if(request->contentLength()){
    debugD("_CONTENT_TYPE: %s\n", request->contentType().c_str());
    debugD("_CONTENT_LENGTH: %u\n", request->contentLength());
  }

  int headers = request->headers();
  int i;
  for(i=0;i<headers;i++){
    AsyncWebHeader* h = request->getHeader(i);
    debugD("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
  }

  int params = request->params();
  for(i=0;i<params;i++){
    AsyncWebParameter* p = request->getParam(i);
    if(p->isFile()){
      debugD("_FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
    } else if(p->isPost()){
      debugD("_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
    } else {
      debugD("_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
    }
  }

  request->send(404);
}

void HTTPServer::handle_config_list(AsyncWebServerRequest* request) {
  AsyncResponseStream *response = request->beginResponseStream("application/json");
  DynamicJsonBuffer json_buffer;
  JsonObject& root = json_buffer.createObject();
  JsonArray& arr = root.createNestedArray("keys");
  for (auto it = configurables.begin(); it!=configurables.end(); ++it) {
    arr.add(it->first);
  }
  root.printTo(*response);
  request->send(response);
}

void HTTPServer::handle_config(AsyncWebServerRequest* request) {
  debugD("%s", request->url().c_str());
  request->send(200, "text/plain", "/config");
}

void HTTPServer::handle_device_reset(AsyncWebServerRequest* request) {
  request->send(200, "text/plain",
    "OK, resetting the device settings back to factory defaults.\n");
  app.onDelay(500, [this](){ this->reset_device(); });
}

void HTTPServer::handle_device_restart(AsyncWebServerRequest* request) {
  request->send(200, "text/plain", "OK, restarting\n");
  app.onDelay(50, [](){ ESP.restart(); });
}


void HTTPServer::handle_info(AsyncWebServerRequest* request) {
  request->send(200, "text/plain", "/info");
}

