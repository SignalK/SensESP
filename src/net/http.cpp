#include "http.h"

#include <functional>

#include <FS.h>
#ifdef ESP32
#include "SPIFFS.h"
#endif

#include <ESPAsyncWebServer.h>

#include "config.h"

// Simple web page to view deltas
const char INDEX_PAGE[] PROGMEM = R"foo(
<html>
<head>
  <title>Deltas</title>
  <meta charset="utf-8">
  <script type="text/javascript">
    var WebSocket = WebSocket || MozWebSocket;
    var lastDelta = Date.now();
    var serverUrl = "ws://" + window.location.hostname + ":81";

    connection = new WebSocket(serverUrl);

    connection.onopen = function(evt) {
      console.log("Connected!");
      document.getElementById("box").innerHTML = "Connected!";
      document.getElementById("last").innerHTML = "Last: N/A";
    };

    connection.onmessage = function(evt) {
      var msg = JSON.parse(evt.data);
      document.getElementById("box").innerHTML = JSON.stringify(msg, null, 2);
      document.getElementById("last").innerHTML = "Last: " + ((Date.now() - lastDelta)/1000).toFixed(2) + " seconds";
      lastDelta = Date.now();
    };

    setInterval(function(){
      document.getElementById("age").innerHTML = "Age: " + ((Date.now() - lastDelta)/1000).toFixed(1) + " seconds";
    }, 50);
  </script>
</head>
<body>
  <h3>Last Delta</h3>
  <pre width="100%" height="50%" id="box">Not Connected yet</pre>
  <div id="last"></div>
  <div id="age"></div>
</body>
</html>
)foo";

HTTPServer::HTTPServer() {
  server = new AsyncWebServer(HTTP_SERVER_PORT);
  using std::placeholders::_1;
  server->onNotFound(std::bind(&HTTPServer::handle_not_found, this, _1));
  server->on("/",[](AsyncWebServerRequest *request ) {
      request->send_P(200, "text/html", INDEX_PAGE);
    });
  server->on("/config", HTTP_GET|HTTP_PUT,
             std::bind(&HTTPServer::handle_config, this, _1));
  server->on("/device/reset", HTTP_GET,
             std::bind(&HTTPServer::handle_device_reset, this, _1));
  server->on("/device/restart", HTTP_GET,
             std::bind(&HTTPServer::handle_device_restart, this, _1));
  server->on("/info", HTTP_GET,
             std::bind(&HTTPServer::handle_info, this, _1));
  }

void HTTPServer::handle_not_found(AsyncWebServerRequest* request) {
  Serial.printf("NOT_FOUND: ");
  if(request->method() == HTTP_GET)
    Serial.printf("GET");
  else if(request->method() == HTTP_POST)
    Serial.printf("POST");
  else if(request->method() == HTTP_DELETE)
    Serial.printf("DELETE");
  else if(request->method() == HTTP_PUT)
    Serial.printf("PUT");
  else if(request->method() == HTTP_PATCH)
    Serial.printf("PATCH");
  else if(request->method() == HTTP_HEAD)
    Serial.printf("HEAD");
  else if(request->method() == HTTP_OPTIONS)
    Serial.printf("OPTIONS");
  else
    Serial.printf("UNKNOWN");
  Serial.printf(" http://%s%s\n", request->host().c_str(), request->url().c_str());

  if(request->contentLength()){
    Serial.printf("_CONTENT_TYPE: %s\n", request->contentType().c_str());
    Serial.printf("_CONTENT_LENGTH: %u\n", request->contentLength());
  }

  int headers = request->headers();
  int i;
  for(i=0;i<headers;i++){
    AsyncWebHeader* h = request->getHeader(i);
    Serial.printf("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
  }

  int params = request->params();
  for(i=0;i<params;i++){
    AsyncWebParameter* p = request->getParam(i);
    if(p->isFile()){
      Serial.printf("_FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
    } else if(p->isPost()){
      Serial.printf("_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
    } else {
      Serial.printf("_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
    }
  }

  request->send(404);
}

void HTTPServer::handle_config(AsyncWebServerRequest* request) {
  request->send(200, "text/plain", "/config");
}

void HTTPServer::handle_device_reset(AsyncWebServerRequest* request) {
  request->send(200, "text/plain", "/device/reset");
}

void HTTPServer::handle_device_restart(AsyncWebServerRequest* request) {
  request->send(200, "text/plain", "/device/restart");
}

void HTTPServer::handle_info(AsyncWebServerRequest* request) {
  request->send(200, "text/plain", "/info");
}
