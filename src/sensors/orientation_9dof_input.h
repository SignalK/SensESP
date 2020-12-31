#ifndef SENSORS_ORIENTATION_9DOF_INPUT_H_
#define SENSORS_ORIENTATION_9DOF_INPUT_H_

#include "sensor.h"
#include "sensor_nxp_fxos8700_fxas21002.h"

/** 
 *  @brief Represents an NXP FXOS8700 accelerometer and magnetometer
 *  sensor, plus an NXP FXAS21002C gyroscope.
 * 
 *  This combination sensor is found
 *  on products such as the Adafruit #3463 breakout board.
 *  It's desirable to capture all the orientation readings (i.e. mag, accel, and
 *  gyro) simultaneously, so the Read9DOF class polls the FXOS8700 and FXAS21002
 *  together, passes the raw values to the Adafruit AHRS / NXP SensorFusion
 *  filter, and stores the combined 9DOF orientation data.
 *  Calling the public Adafruit_FXOS8700:: methods can be done
 *  after you instantiate Orientation9DOF, for example by:
 
    sensor_fxos_fxas->pAdafruitFXOS8700->setSampling();
  
 *  See the Adafruit library for details.
 *  https://github.com/adafruit/Adafruit_FXOS8700_Library/blob/master/Adafruit_FXOS8700.h
 * 
 *  @param pin_i2c_sda The pin you're using for I2C SDA.
 * 
 *  @param pin_i2c_scl The pin you're using for I2c SCL.
**/
class Orientation9DOF {
 public:
  Orientation9DOF(uint8_t pin_i2c_sda, uint8_t pin_i2c_scl);
  void stream_raw_values(void);  // used when calibrating
  // pointer to physical sensor
  SensorNXP_FXOS8700_FXAS21002 *sensor_fxos_fxas_;
};

/**
 * @brief sets up the combo FXOS8700 + FXAS21002 sensor and outputs the
 * specified orientation parameter value.
 * 
 * Setting up the sensor combo FXOS8700 + FXAS21002 includes loading the
 * configuration, initializing the filter, and calibrating the sensor.
 * 
 * FIXME: The above description is not adequate for an end user to know
 * how to use this sensor properly. Those details are in the .cpp file, but
 * they need to be here in order to appear in the Doxygen documentation.
 * 
 * @param p9DOF Pointer to an Orientation9DOF.
 * 
 * @param val_type The type of value you want to read, from the enum
 * OrientationValType. Defaults to compass_hdg.
 * 
 * @param read_delay How often you want to read the value, in ms. Default is 100.
 * 
 * @param config_path The path to configure the sensor in the Config UI.
 **/
class Read9DOF : public NumericSensor {
 public:
  enum OrientationValType {
  compass_hdg,
  pitch,
  roll,
  acceleration_x,
  acceleration_y,
  acceleration_z,
  rate_of_turn,
  rate_of_pitch,
  rate_of_roll
};
  Read9DOF(Orientation9DOF* p9DOF, OrientationValType val_type = compass_hdg,
           uint read_delay = 100, String config_path = "");
  void enable() override final;
  Orientation9DOF* orientation_9dof_;

 private:
  OrientationValType val_type_;
  uint read_delay_;
  void update(void);
  virtual void get_configuration(JsonObject& doc) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;
};

#endif
