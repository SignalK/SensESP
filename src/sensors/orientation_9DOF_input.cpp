#include "orientation_9DOF_input.h"

#include <RemoteDebug.h>
#include "sensesp.h"

// pointer to physical sensor
SensorNXP_FXOS8700_FXAS21002*
    pSensorFXOSFXAS;  // if pSensorFXOSFXAS is a member of Orientation9DOF, then
                      // when called in onRepeat it causes CPU panic 

// Orientation9DOF represents a 9-Degrees-of-Freedom sensor (magnetometer,
// accelerometer, and gyroscope), such as an
// ADAfruit NXP FXOS8700 + FXAS21002 combination sensor.
// TODO - make accel range a configurable
Orientation9DOF::Orientation9DOF(uint8_t pin_i2c_sda, uint8_t pin_i2c_scl,
                                 String config_path)
    : Sensor(config_path) {
  className = "Orientation9DOF";
  load_configuration(); //no effect, since get_ and set_configuration() not defined
  pSensorFXOSFXAS = new SensorNXP_FXOS8700_FXAS21002();
  if (!pSensorFXOSFXAS->connect(pin_i2c_sda, pin_i2c_scl)) {
    debugE(
        "No connection to FXOS8700/FXAS21002 sensor: check address & wiring");
  }
}

void Orientation9DOF::displaySensorDetails(void) {
  pSensorFXOSFXAS->printSensorDetails();
}

void Orientation9DOF::streamRawValues(void) {
  // Used only when calibrating. This method does not return.
  debugI("calling gatherCalibrationData()");
  while (true) {
    pSensorFXOSFXAS->gatherCalibrationDataOnce(true);
    delay(10);
  }
}

// Read9DOF() sets up access to the combo FXOS8700 + FXAS21002 sensor, loads
// its configuration, and initializes the filter that turns raw data into 
// desired orientation parameter. For accurate filter output, sensor needs 
// to be calibrated first. OrientationValType val_type specifies what kind of
// orientation parameter value is to be read (heading, linear accel, or angular
// velocity)
Read9DOF::Read9DOF(Orientation9DOF* p9DOF, OrientationValType val_type,
                   uint read_delay, String config_path)
    : NumericSensor(config_path),
      pOrientation9DOF{p9DOF},
      val_type{val_type},
      read_delay{read_delay} {
  className = "Read9DOF";
  load_configuration();
  pSensorFXOSFXAS->initFilter(this->read_delay);
}

// Setup repeated readings from combination sensor.
void Read9DOF::enable() {
  // some sensor examples implement the sensor readings directly in this
  // enable() method, rather than by calling update(). Is there a reason to
  // prefer one method over the other?
  app.onRepeat(read_delay, [this]() { this->update(); });
}

// Obtain one set of readings from combination sensor. Values are updated from
// all three modes (magnetometer, accelerometer, gyro)
// but only the requested val_type is returned in the output variable.
// TODO - how to return multiple values
void Read9DOF::update() {
  // if pSensorFXOSFXAS is a member of Orientation9DOF, then when called in
  // onRepeat it causes CPU panic
  pSensorFXOSFXAS->gatherOrientationDataOnce(false);
  switch (val_type) {
    case (accelerometer):
      // TODO - process full return vals. Right now does nothing useful.
      output = -1.0;
      break;
    case (compass_hdg):
      output = pSensorFXOSFXAS->getHeadingRadians();
      break;
    case (gyro):
      // TODO - process full return vals. Right now does nothing useful.
      output = -2.0;
      break;
    default:
      output = -3.0;
  }
  notify();
}

JsonObject& Read9DOF::get_configuration(JsonBuffer& buf) {
  JsonObject& root = buf.createObject();
  root["read_delay"] = read_delay;
  root["value"] = output;
  return root;
};

static const char SCHEMA[] PROGMEM = R"###({
    "type": "object",
    "properties": {
        "read_delay": { "title": "Read delay", "type": "number", "description": "The time, in milliseconds, between each read of the input" },
        "value": { "title": "Last value", "type" : "number", "readOnly": true }
    }
  })###";

String Read9DOF::get_config_schema() { return FPSTR(SCHEMA); }

bool Read9DOF::set_configuration(const JsonObject& config) {
  String expected[] = {"read_delay"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  read_delay = config["read_delay"];
  return true;
}
