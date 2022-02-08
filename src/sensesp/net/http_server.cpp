#include "http_server.h"

#include <ESPAsyncWebServer.h>
#include <FS.h>

#include <functional>
#include <map>

#include "ArduinoJson.h"
#include "AsyncJson.h"
#include "sensesp/system/configurable.h"
#include "sensesp/system/ui_output.h"
#include "sensesp_base_app.h"

// Include the web UI stored in PROGMEM space
#include "web/css_bootstrap.h"
#include "web/index.h"
#include "web/js_jsoneditor.h"
#include "web/js_sensesp.h"

namespace sensesp {

// HTTP port for the configuration interface
#ifndef HTTP_SERVER_PORT
#define HTTP_SERVER_PORT 80
#endif

std::map<String, HTTPCommand*> http_commands;

HTTPServer::HTTPServer() : Startable(50) {
  server = new AsyncWebServer(HTTP_SERVER_PORT);
  using std::placeholders::_1;

  server->onNotFound(std::bind(&HTTPServer::handle_not_found, this, _1));

  // Handle setting configuration values of a Configurable via a Json PUT to
  // /config
  AsyncCallbackJsonWebHandler* config_put_handler =
      new AsyncCallbackJsonWebHandler(
          "/config", [](AsyncWebServerRequest* request, JsonVariant& json) {
            // omit the "/config" part of the url
            String url_tail = request->url().substring(7);

            if (url_tail == "") {
              request->send(405, "text/plain",
                            F("PUT to /config not allowed.\n"));
              return;
            }

            std::map<String, Configurable*>::iterator it =
                configurables.find(url_tail);
            if (it == configurables.end()) {
              request->send(404, "text/plain",
                            F("Configuration key not found.\n"));
              return;
            }
            Configurable* confable = it->second;

            JsonObject body = json.as<JsonObject>();

            if (!confable->set_configuration(body)) {
              request->send(400, "text/plain",
                            F("Unable to extract keys from JSON.\n"));
              return;
            }
            confable->save_configuration();
            request->send(200, "text/plain", F("Configuration successful.\n"));
            return;
          });
  config_put_handler->setMethod(HTTP_PUT);
  server->addHandler(config_put_handler);

  // Handle requests to retrieve the current Json configuration of a
  // Configurable via HTTP GET on /config
  server->on("/config", HTTP_GET, [this](AsyncWebServerRequest* request) {
    // omit the "/config" part of the url
    String url_tail = request->url().substring(7);

    if (url_tail == "" || url_tail == "/") {
      this->handle_config_list(request);
      return;
    }

    std::map<String, Configurable*>::iterator it = configurables.find(url_tail);
    if (it == configurables.end()) {
      request->send(404, "text/plain", F("Configuration key not found.\n"));
      return;
    }
    Configurable* confable = it->second;

    AsyncResponseStream* response =
        request->beginResponseStream("application/json");
    DynamicJsonDocument json_doc(1024);
    JsonObject config = json_doc.createNestedObject("config");
    confable->get_configuration(config);
    json_doc["schema"] = serialized(confable->get_config_schema());
    json_doc["description"] = confable->get_config_description();
    serializeJson(json_doc, *response);
    request->send(response);
  });

  server->on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    debugD("Serving gziped index.html");
    AsyncWebServerResponse* response = request->beginResponse_P(
        200, "text/html", PAGE_index, PAGE_index_size, NULL);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  server->on(
      "/css/bootstrap.min.css", HTTP_GET, [](AsyncWebServerRequest* request) {
        debugD("Serving gziped bootstrap.min.css");
        AsyncWebServerResponse* response = request->beginResponse_P(
            200, "text/css", PAGE_css_bootstrap, PAGE_css_bootstrap_size);
        response->addHeader("Content-Encoding", "gzip");
        request->send(response);
      });

  server->on(
      "/css/bootstrap.min.css", HTTP_GET, [](AsyncWebServerRequest* request) {
        debugD("Serving gziped bootstrap.min.css");
        AsyncWebServerResponse* response = request->beginResponse_P(
            200, "text/css", PAGE_css_bootstrap, PAGE_css_bootstrap_size);
        response->addHeader("Content-Encoding", "gzip");
        request->send(response);
      });

  server->on(
      "/js/jsoneditor.min.js", HTTP_GET, [](AsyncWebServerRequest* request) {
        debugD("Serving gziped jsoneditor.min.js");
        AsyncWebServerResponse* response = request->beginResponse_P(
            200, "text/html", PAGE_js_jsoneditor, PAGE_js_jsoneditor_size);
        response->addHeader("Content-Encoding", "gzip");
        request->send(response);
      });

  server->on("/js/sensesp.js", HTTP_GET, [](AsyncWebServerRequest* request) {
    debugD("Serving gziped sensesp.js");
    AsyncWebServerResponse* response = request->beginResponse_P(
        200, "text/javascript", PAGE_js_sensesp, PAGE_js_jsoneditor_size);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  server->on("/device/reset", HTTP_GET,
             std::bind(&HTTPServer::handle_device_reset, this, _1));
  server->on("/device/restart", HTTP_GET,
             std::bind(&HTTPServer::handle_device_restart, this, _1));
  server->on("/info", HTTP_GET, std::bind(&HTTPServer::handle_info, this, _1));

  server->on("/command", HTTP_GET,
             std::bind(&HTTPServer::handle_command, this, _1));
}

void HTTPServer::handle_not_found(AsyncWebServerRequest* request) {
  debugD("NOT_FOUND: ");
  if (request->method() == HTTP_GET) {
    debugD("GET");
  } else if (request->method() == HTTP_POST) {
    debugD("POST");
  } else if (request->method() == HTTP_DELETE) {
    debugD("DELETE");
  } else if (request->method() == HTTP_PUT) {
    debugD("PUT");
  } else if (request->method() == HTTP_PATCH) {
    debugD("PATCH");
  } else if (request->method() == HTTP_HEAD) {
    debugD("HEAD");
  } else if (request->method() == HTTP_OPTIONS) {
    debugD("OPTIONS");
  } else {
    debugD("UNKNOWN");
  }
  debugD(" http://%s%s\n", request->host().c_str(), request->url().c_str());

  if (request->contentLength()) {
    debugD("_CONTENT_TYPE: %s\n", request->contentType().c_str());
    debugD("_CONTENT_LENGTH: %u\n", request->contentLength());
  }

  int headers = request->headers();
  for (int i = 0; i < headers; i++) {
    AsyncWebHeader* h = request->getHeader(i);
    debugD("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
  }

  int params = request->params();
  for (int i = 0; i < params; i++) {
    AsyncWebParameter* p = request->getParam(i);
    if (p->isFile()) {
      debugD("_FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(),
             p->size());
    } else if (p->isPost()) {
      debugD("_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
    } else {
      debugD("_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
    }
  }

  request->send(404);
}

void HTTPServer::handle_config_list(AsyncWebServerRequest* request) {
  // to save memory, guesstimate the required output buffer size based
  // on the number of elements
  auto output_buffer_size = 200 * configurables.size();
  AsyncResponseStream* response =
      request->beginResponseStream("application/json");
  DynamicJsonDocument json_doc(output_buffer_size);
  JsonArray arr = json_doc.createNestedArray("keys");
  for (auto it = configurables.begin(); it != configurables.end(); ++it) {
    arr.add(it->first);
  }
  serializeJson(json_doc, *response);
  request->send(response);
}

void HTTPServer::handle_config(AsyncWebServerRequest* request) {
  debugD("%s", request->url().c_str());
  request->send(200, "text/plain", "/config");
}

void HTTPServer::handle_device_reset(AsyncWebServerRequest* request) {
  request->send(
      200, "text/plain",
      "OK, resetting the device settings back to factory defaults.\n");
  ReactESP::app->onDelay(500, [this]() { SensESPBaseApp::get()->reset(); });
}

void HTTPServer::handle_device_restart(AsyncWebServerRequest* request) {
  request->send(200, "text/plain", "OK, restarting\n");
  ReactESP::app->onDelay(50, []() { ESP.restart(); });
}

void HTTPServer::handle_info(AsyncWebServerRequest* request) {
  auto* response = request->beginResponseStream("application/json");
  response->setCode(200);
  auto output_buffer_size = (200 * configurables.size()) + 512;
  DynamicJsonDocument json_doc(output_buffer_size);
  auto properties = json_doc.createNestedObject("Properties");
  auto commands = json_doc.createNestedArray("Commands");
  auto pages = json_doc.createNestedArray("Pages");
  auto config = json_doc.createNestedArray("Config");

  for (auto property = ui_outputs.begin(); property != ui_outputs.end();
       ++property) {
    property->second->set_json(properties);
  }
  // add all configuration paths
  for (auto it = configurables.begin(); it != configurables.end(); ++it) {
    config.add(it->first);
  }

  for (auto command = http_commands.begin(); command != http_commands.end();
       ++command) {
    auto jCommand = commands.createNestedObject();
    jCommand["Name"] = command->second->get_name();
    jCommand["Title"] = command->second->get_title();
    jCommand["Confirm"] = command->second->get_mustConfirm();
  }

  serializeJson(json_doc, *response);
  request->send(response);
}

void HTTPServer::handle_command(AsyncWebServerRequest* request) {
  if (request->hasParam("id")) {
    auto id = request->getParam("id")->value();
    auto command = http_commands.find(id);

    if (command != http_commands.end()) {
      debugI("Handle command %s id=%s", request->url().c_str(), id.c_str());
      command->second->notify();
      request->send(200, "text/html", "Success!");
    } else {
      request->send(404, "text/html", "Command not found!");
    }
  } else {
    request->send(400, "text/html", "Missing id parameter.");
  }
}

HTTPCommand* HTTPServer::add_command(String name, String title,
                                     bool mustConfirm) {
  auto ret = new HTTPCommand(title, name, mustConfirm);

  http_commands[name] = ret;

  return ret;
}

}  // namespace sensesp
