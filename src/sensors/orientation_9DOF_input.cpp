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
Orientation9DOF::Orientation9DOF(uint8_t pin_i2c_sda, uint8_t pin_i2c_scl,
                                 String config_path)
    : Sensor(config_path) {
  className = "Orientation9DOF";
  load_configuration();
  pSensorFXOSFXAS = new SensorNXP_FXOS8700_FXAS21002();
  if (!pSensorFXOSFXAS->connect(pin_i2c_sda, pin_i2c_scl)) {
    debugE(
        "No connection to FXOS8700/FXAS21002 sensor: check address & wiring");
  }else {
    pSensorFXOSFXAS->printSensorDetails();
  }

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
  app.onRepeat(read_delay, [this]() { this->update(); });
}

// Provides one parameter reading from the combination sensor. 
// val_type determines which particular parameter is output.
// Note that since the filter algorithm works best when all 9-DOF
//  parameters are captured simultaneously, and at a specified
//  constant rate, there is only a single call to 
//  pSensorFXOSFXAS->gatherOrientationDataOnce() in this method.
//  Arbitrarily, this update happens when the compass heading
//  is requested; all the remaining parameters are retrieved
//  by calling the appropriate pSensorFXOSFXAS->getter method.
// If the compass heading parameter is not the one that is set
//  to the fastest repetition rate (in main.cpp when calling
//  Read9DOF::Read9DOF()), then you should move the  
//  gatherOrientationDataOnce() call into the case block
//  for whichever parameter _is_ updated the fastest - this
//  ensures that that parameter and all others are never stale.
void Read9DOF::update() {
  // if pSensorFXOSFXAS is a member of Orientation9DOF, then when called in
  // onRepeat it causes CPU panic
  switch (val_type) {
    case (compass_hdg):
      //sensor is read and filter called, only for compass_hdg
      //remaining parameters are obtained from most recent filter results
      pSensorFXOSFXAS->gatherOrientationDataOnce(false);
      output = pSensorFXOSFXAS->getHeadingRadians();
      break;
    case (roll):
      output = pSensorFXOSFXAS->getRollRadians();
      break;
    case (pitch):
      output = pSensorFXOSFXAS->getPitchRadians();
      break;
    case (acceleration_x):
      output = pSensorFXOSFXAS->getAccelerationX();
      break;
    case (acceleration_y):
      output = pSensorFXOSFXAS->getAccelerationY();
      break;
    case (acceleration_z):
      output = pSensorFXOSFXAS->getAccelerationZ();
      break;
    case (rate_of_turn):
      output = pSensorFXOSFXAS->getRateOfTurn();
      break;
    case (rate_of_pitch):
      output = pSensorFXOSFXAS->getRateOfPitch();
      break;
    case (rate_of_roll):
      output = pSensorFXOSFXAS->getRateOfRoll();
      break;
    default:
      output = 0.0;
  }
  notify();
}

void Read9DOF::get_configuration(JsonObject& doc) {
  doc["read_delay"] = read_delay;
  doc["value"] = output;
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
