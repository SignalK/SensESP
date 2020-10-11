#ifndef SENSORS_ORIENTATION_9DOF_INPUT_H_
#define SENSORS_ORIENTATION_9DOF_INPUT_H_

//  SensESP Sensor definition file.
//  Provides Orientation from 9DOF sensor combination (magnetometer, 
//    accelerometer, gyroscope) consisting of FXOS8700 + FXAS21002
 
#include "sensor.h"
#include "sensor_nxp_fxos8700_fxas21002.h"

// Orientation9DOF represents an NXP FXOS8700 accelerometer and magnetometer
//  sensor, plus an NXP FXAS21002C gyroscope. This combination sensor is found
//  on products such as the Adafruit #3463 breakout board.
//
// It's desirable to capture all the orientation readings (i.e. mag, accel, and
//  gyro) simultaneously, so the class Read9DOF polls the FXOS8700 and FXAS21002
//  together, passes the raw values to the Adafruit AHRS / NXP SensorFusion
//  filter, and stores the combined 9DOF orientation data.
// Calling the public Adafruit_FXOS8700:: methods can be done
//  after you instantiate Orientation9DOF, for example by:
//  sensor_fxos_fxas->pAdafruitFXOS8700->setSampling();
//  See the Adafruit library for details.
//  https://github.com/adafruit/Adafruit_FXOS8700_Library/blob/master/Adafruit_FXOS8700.h
class Orientation9DOF : public Sensor {
 public:
  Orientation9DOF(uint8_t pin_i2c_sda, uint8_t pin_i2c_scl,
                  String config_path = "");
  void stream_raw_values(void);       // used when calibrating
 private:
  uint8_t addr;  // unused
};

// Pass one of these in Read9DOF() constructor corresponding to type of value you want
enum OrientationValType {
  compass_hdg, pitch, roll, 
  acceleration_x, acceleration_y, acceleration_z,
  rate_of_turn, rate_of_pitch, rate_of_roll
};

// Read9DOF reads the combo FXOS8700 + FXAS21002 sensor and outputs the
// specified orientation parameter value
class Read9DOF : public NumericSensor {
 public:
  Read9DOF(Orientation9DOF* p9DOF, OrientationValType val_type = compass_hdg,
           uint read_delay = 100, String config_path = "");
  void enable() override final;
  Orientation9DOF* orientation_9dof;

 private:
  OrientationValType val_type;
  uint read_delay;
  void update(void);  // doesn't exist in some examples (e.g. BME280)
                      // Incorporated into enable() instead.
  virtual void get_configuration(JsonObject& doc) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;
};

#endif  //SENSORS_ORIENTATION_9DOF_INPUT_H_