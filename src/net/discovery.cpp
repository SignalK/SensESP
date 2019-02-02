#ifdef ESP8266
  #include <ESP8266mDNS.h>        // Include the mDNS library
#elif defined(ESP32)
  #include <ESPmDNS.h>
#endif


void setup_discovery(const char* hostname) {
  if (!MDNS.begin(hostname)) {             // Start the mDNS responder for esp8266.local
    Serial.println(F("Error setting up mDNS responder"));
  } else {
    Serial.print (F("mDNS responder started at "));
    Serial.print (hostname);
    Serial.println(F(""));
  }
  MDNS.addService("http", "tcp", 80);
}
