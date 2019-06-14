
#include "signalk_position.h"

template<>
String SKOutput<Position>::as_signalK() {
    DynamicJsonBuffer jsonBuffer;
    String json;
    JsonObject& root = jsonBuffer.createObject();
    root.set("path", this->get_sk_path());
    JsonObject& value = root.createNestedObject("value");
    value.set("latitude", output.latitude);
    value.set("longitude", output.longitude);
    if (output.altitude > -10000) {
      value.set("altitude", output.altitude);
    }
    root.printTo(json);
    return json;
}
