/** @file orientation_sensor.h
 *  @brief Orientation sensor interface to SensESP
 * 
 * Provides Orientation from 9DOF sensor combination (magnetometer,
 * accelerometer, gyroscope) consisting of FXOS8700 + FXAS21002
 */

#ifndef orientation_sensor_H_
#define orientation_sensor_H_

#include "sensor_fusion_class.h"  // for OrientationSensorFusion-ESP library

#include "sensors/sensor.h"
#include "signalk/signalk_attitude.h"

/**
 * @brief OrientationSensor represents a 9-Degrees-of-Freedom sensor
 * (magnetometer, accelerometer, and gyroscope).
 *
 * This class provides the interface to the SensorFusion library which performs
 * the I2C communication with the sensor and runs the sensor fusion algorithm.
 *
 * A compatible sensor is the NXP FXOS8700 + FXAS21002 combination sensor.
 * This combination sensor is found on products such as the
 * Adafruit #3463 breakout board. The OrientationSensorFusion-ESP
 * library is configured to use this NXP sensor by default, though
 * other sensors can be used by adjusting the library's build.h
 * and board.h files. Calling the public SensorFusion:: methods
 * can be done after you instantiate OrientationSensor, for example by:
 * orientation_sensor->sensor_interface_->GetOrientationQuaternion();
 * The OrientationSensorFusion-ESP library has details:
 * @see https://github.com/BjarneBitscrambler/OrientationSensorFusion-ESP.git
  */
class OrientationSensor {
 public:
  OrientationSensor(uint8_t pin_i2c_sda, uint8_t pin_i2c_scl,
                    uint8_t accel_mag_i2c_addr, uint8_t gyro_i2c_addr,
                    String config_path = "");
  SensorFusion* sensor_interface_; ///< sensor's Fusion Library interface
 
 private:
  void ReadAndProcessSensors(void);  ///< reads sensor and runs fusion algorithm
};

/**
 * @brief AttitudeValues reads and outputs attitude (yaw,pitch,roll) parameters.
 *
 * The three parameters are stored in an Attitude struct, and sent together
 * in one Signal K message. The units are radians.
 */
class AttitudeValues : public AttitudeProducer, public Sensor {
 public:
  AttitudeValues(OrientationSensor* orientation_sensor,
                 uint read_delay_ms = 100, String config_path = "");
  void enable() override final;  ///< starts periodic outputs of Attitude
  OrientationSensor*
      orientation_sensor_;  ///< Pointer to the orientation sensor

 private:
  void Update(void);  ///< fetches current attitude and notifies consumer
  virtual void get_configuration(JsonObject& doc) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;
  Attitude attitude_;   ///< struct storing the current yaw,pitch,roll values
  uint read_delay_ms_;  ///< interval between attitude updates to Signal K

};  // end class AttitudeValues

/**
 * @brief OrientationValues reads and outputs orientation parameters.
 *
 * One parameter is sent per instance of OrientationValues, selected
 * from the list of OrientationValType. The one exception is the
 * attitude (yaw,pitch,roll) which consists of three parameters and
 * is provided by the AttitudeValues class instead of this one.
 * Create new instances in main.cpp for each parameter desired.
 */
class OrientationValues : public NumericSensor {
 public:
  enum OrientationValType {
    kCompassHeading,  ///< compass heading, also called yaw
    kYaw,             ///< rotation about the vertical axis
    kPitch,           ///< rotation about the transverse axis
    kRoll,            ///< rotation about the longitudinal axis
    kAttitude,        ///< attitude combines heading, pitch, and roll
    kAccelerationX,   ///< acceleration in the stern-to-bow axis
    kAccelerationY,   ///< acceleration in the starboard-to-port axis
    kAccelerationZ,   ///< acceleration in the down-to-up axis
    kRateOfTurn,      ///< rate of change of compass heading
    kRateOfPitch,     ///< rate of change of pitch
    kRateOfRoll,      ///< rate of change of roll
    kTemperature      ///< temperature as reported by sensor IC
  };
  OrientationValues(OrientationSensor* orientation_sensor,
                    OrientationValType value_type = kCompassHeading,
                    uint read_delay_ms = 100, String config_path = "");
  void enable() override final;  ///< starts periodic outputs of Attitude
  OrientationSensor*
      orientation_sensor_;  ///< Pointer to the orientation sensor

 private:
  void Update(
      void);  ///< fetches current orientation parameter and notifies consumer
  virtual void get_configuration(JsonObject& doc) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;
  OrientationValType
      value_type_;      ///< Particular type of orientation parameter supplied
  uint read_delay_ms_;  ///< Interval between data outputs via Signal K

};  // end class OrientationValues

#endif  // ORIENTATION_SENSOR_H_
