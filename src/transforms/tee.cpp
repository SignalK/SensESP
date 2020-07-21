#include "tee.h"

Tee::Tee( uint16_t isTeeActive, float coefficient_tap, float offset_tap, void (*pFunc_tap)(float x), String config_path ) : 
          NumericTransform(config_path), 
          isTeeActive{isTeeActive}, coefficient_tap{coefficient_tap}, offset_tap{offset_tap}  {
  className = "Tee";
  load_configuration();
  tee_to = pFunc_tap;
}

void Tee::set_input(float input, uint8_t inputChannel) {

        output = input; //pass input value unchanged to next in chain
         if( isTeeActive )
        {   //route the data to external function, which can e.g. send it to GPIO, or log it, or...
            (*tee_to)( (float)input * coefficient_tap + offset_tap); //apply y = mx + b linear equation to input
        }
        notify();  
}

JsonObject& Tee::get_configuration(JsonBuffer& buf) {
  JsonObject& root = buf.createObject();
  root["coefficient_tap"] = coefficient_tap;
  root["offset_tap"] = offset_tap;
  root["isTeeActive"] = isTeeActive;
  debugI( "Tee get_configuration: coefficient_tap=%f offset_tap=%f isTeeActive=%d", 
          coefficient_tap, offset_tap, isTeeActive );
  return root;
}

static const char SCHEMA[] PROGMEM = R"###({
    "type": "object",
    "properties": {
       "coefficient_tap": { "title": "Coefficient m in y=mx + b", "description": "converts Alternator signal (Hz) into Tachometer Signal (Hz)", "type": "number" },
       "offset_tap": { "title": "Offset b in y=mx + b", "type": "number" },
       "isTeeActive": { "title": "Tee Active", "description": "Whether value passing through is echoed to GPIO. 0 = no passthrough.", "type": "number" }
    }
  })###"; //some examples lack the ### - what are they for? Are they needed?

String Tee::get_config_schema() {
  return FPSTR(SCHEMA);
}


bool Tee::set_configuration(const JsonObject& config) {
  String expected[] = {"isTeeActive", "coefficient_tap", "offset_tap" };
  for (auto str : expected) {
    if (!config.containsKey(str)) 
    { debugI( "Tee set_configuration: couldn't find %s", str.c_str() );
      return false;
    }
  }
  isTeeActive = config["isTeeActive"];
  coefficient_tap = config["coefficient_tap"];
  offset_tap = config["offset_tap"];
  debugI( "Tee set_configuration: coefficient_tap=%f offset_tap=%f isTeeActive=%d", coefficient_tap, offset_tap, isTeeActive );
  return true;
}
