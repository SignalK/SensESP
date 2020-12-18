/** @file orientation_sensor.cpp
 *  @brief Orientation sensor interface to SensESP
 */

#include "sensors/orientation_sensor.h"

#include <RemoteDebug.h>

#include "sensesp.h"

/**
 * @brief Constructor sets up the I2C communications to the sensor and
 * initializes the sensor fusion library.
 *
 * @param pin_i2c_sda Pin of SDA line to sensor. Use -1 for Arduino default.
 * @param pin_i2c_scl Pin of SCL line to sensor. Use -1 for Arduino default.
 * @param accel_mag_i2c_addr I2C address of accelerometer/magnetometer IC.
 * @param gyro_i2c_addr I2C address of gyroscope IC.
 * @param config_path RESTful path by which sensor can be configured.
 */
OrientationSensor::OrientationSensor(uint8_t pin_i2c_sda, uint8_t pin_i2c_scl,
                                     uint8_t accel_mag_i2c_addr,
                                     uint8_t gyro_i2c_addr, String config_path)
    : Sensor(config_path) {
  load_configuration();
  sensor_interface_ = new SensorFusion();  // create our fusion engine instance

  bool success;
  // init IO subsystem, passing NULLs since we use Signal-K output instead.
  success =
      sensor_interface_->InitializeInputOutputSubsystem(NULL, NULL) &&
      // connect to the sensors.  Accelerometer and magnetometer are in same IC.
      sensor_interface_->InstallSensor(accel_mag_i2c_addr,
                                       SensorType::kMagnetometer) &&
      sensor_interface_->InstallSensor(accel_mag_i2c_addr,
                                       SensorType::kAccelerometer) &&
      // A thermometer (uncalibrated) is available in the
      // accelerometer/magnetometer IC.
      sensor_interface_->InstallSensor(accel_mag_i2c_addr,
                                       SensorType::kThermometer) &&
      sensor_interface_->InstallSensor(gyro_i2c_addr, SensorType::kGyroscope);
  if( ! success ) {
    debugE("Trouble installing sensors.");
  } else {
    sensor_interface_->Begin(pin_i2c_sda, pin_i2c_scl);
    debugI("Sensors connected & Fusion ready");
  }

}  // end OrientationSensor()

/**
 * @brief Starts periodic reads of sensor and running of fusion algorithm.
 *
 * The enable() function is inherited from Sensor::, and is
 * automatically called when the SensESP app starts.
 */
void OrientationSensor::enable(void) {
  // The Fusion Library, in build.h, defines how fast the ICs generate new
  // orientation data and how fast the fusion algorithm runs, using FUSION_HZ.
  // Usually this rate should be the same as ReadAndProcessSensors() is called.
  const uint32_t kFusionIntervalMs = 1000.0 / FUSION_HZ;
  app.onRepeat(kFusionIntervalMs, [this]() { this->ReadAndProcessSensors(); });
}  // end Enable()

/**
 * @brief Read the Sensors and calculate orientation parameters
 */
void OrientationSensor::ReadAndProcessSensors(void) {
  sensor_interface_->ReadSensors();
  sensor_interface_->RunFusion();

}  // end ReadAndProcessSensors()

/**
 * @brief Constructor sets up the frequency of output and the Signal K path.
 *
 * @param orientation_sensor Pointer to the physical sensor's interface
 * @param read_delay_ms Interval between output reports
 * @param config_path RESTful path by which reporting frequency can be
 * configured.
 */
AttitudeValues::AttitudeValues(OrientationSensor* orientation_sensor,
                               uint read_delay_ms, String config_path)
    : orientation_sensor_{orientation_sensor}, read_delay_ms_{read_delay_ms} {
  load_configuration();

}  // end AttitudeValues()

/**
 * @brief Starts periodic output of Attitude parameters.
 *
 * The enable() function is inherited from Sensor::, and is
 * automatically called when the SensESP app starts.
 */
void AttitudeValues::enable() {
  app.onRepeat(read_delay_ms_, [this]() { this->Update(); });
}

/**
 * @brief Provides one Attitude reading from the orientation sensor.
 *
 * Readings are obtained using the sensor fusion library's Get_() methods
 * and assigned to the output variable that passes data from Producers
 * to Consumers. Consumers of the attitude data are then informed
 * by the call to notify(). If data are not valid (e.g. sensor not
 * functioning), a struct member is set to false so when the Signal K
 * message contents are assembled by as_signalk(),they can reflect that. 
 */
void AttitudeValues::Update() {
  attitude_.is_data_valid =
      orientation_sensor_->sensor_interface_->IsDataValid();
  attitude_.yaw = orientation_sensor_->sensor_interface_->GetHeadingRadians();
  attitude_.roll =
      orientation_sensor_->sensor_interface_->GetRollRadians();
  attitude_.pitch =
      orientation_sensor_->sensor_interface_->GetPitchRadians();

  output = attitude_;
  notify();
}  // end Update()

/**
 * @brief Define the format for the Orientation value producers.
 *
 * This format is common to the single orientation parameter producers
 * (OrientationValues objects) and the attitude parameter producers
 * (AttitudeValues objects).
 */
static const char SCHEMA[] PROGMEM = R"###({
    "type": "object",
    "properties": {
        "read_delay": { 
          "title": "Read delay", 
          "type": "number", 
          "description": "Interval in milliseconds between outputs of this parameter" 
        }
    }
  })###";

/**
 * @brief Get the current sensor configuration and place it in a JSON
 * object that can then be stored in non-volatile memory.
 * 
 * @param doc JSON object to contain the configuration parameters
 * to be updated.
 */
void AttitudeValues::get_configuration(JsonObject& doc) {
  doc["read_delay"] = read_delay_ms_;
}  // end get_configuration()

/**
 * @brief Fetch the JSON format used for holding the configuration.
 */
String AttitudeValues::get_config_schema() { return FPSTR(SCHEMA); }

/**
 * @brief Use the values stored in JSON object config to update
 * the appropriate member variables.
 *
 * @param config JSON object containing the configuration parameters
 * to be updated.
 * @return True if successful; False if a parameter could not be found.
 */
bool AttitudeValues::set_configuration(const JsonObject& config) {
  String expected[] = {"read_delay"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  read_delay_ms_ = config["read_delay"];
  return true;
}  // end set_configuration()

/**
 * @brief Constructor sets up the frequency of output and the Signal K path.
 *
 * @param orientation_sensor Pointer to the physical sensor's interface
 * @param val_type The type of orientation parameter to be sent
 * @param read_delay_ms Interval between output reports
 * @param config_path RESTful path by which reporting frequency can be
 * configured.
 */
OrientationValues::OrientationValues(OrientationSensor* orientation_sensor,
                                     OrientationValType val_type,
                                     uint read_delay_ms, String config_path)
    : NumericSensor(config_path),
      orientation_sensor_{orientation_sensor},
      value_type_{val_type},
      read_delay_ms_{read_delay_ms} {
  load_configuration();

}  // end OrientationValues()

/**
 * @brief Starts periodic output of orientation parameter.
 *
 * The enable() function is inherited from Sensor::, and is
 * automatically called when the SensESP app starts.
 */
void OrientationValues::enable() {
  app.onRepeat(read_delay_ms_, [this]() { this->Update(); });
}

/**
 * @brief Provides one orientation parameter reading from the sensor.
 *
 * value_type_ determines which particular parameter is output.
 * Readings are obtained using the sensor fusion library's get() methods
 * and assigned to the output variable that passes data from Producers
 * to Consumers. Consumers of the orientation data are then informed
 * by the call to notify()
 */
void OrientationValues::Update() {
  switch (value_type_) {
    case (kCompassHeading):
      output = orientation_sensor_->sensor_interface_->GetHeadingRadians();
      break;
    case (kRoll):
      output = orientation_sensor_->sensor_interface_->GetRollRadians();
      break;
    case (kPitch):
      output = orientation_sensor_->sensor_interface_->GetPitchRadians();
      break;
    case (kAccelerationX):
      output = orientation_sensor_->sensor_interface_->GetAccelXMPerSS();
      break;
    case (kAccelerationY):
      output = orientation_sensor_->sensor_interface_->GetAccelYMPerSS();
      break;
    case (kAccelerationZ):
      output = orientation_sensor_->sensor_interface_->GetAccelZMPerSS();
      break;
    case (kRateOfTurn):
      output = orientation_sensor_->sensor_interface_->GetTurnRateRadPerS();
      break;
    case (kRateOfPitch):
      output = orientation_sensor_->sensor_interface_->GetPitchRateRadPerS();
      break;
    case (kRateOfRoll):
      output = orientation_sensor_->sensor_interface_->GetRollRateRadPerS();
      break;
    case (kTemperature):
      output = orientation_sensor_->sensor_interface_->GetTemperatureK();
      break;
    default:
      output = N2K_INVALID_FLOAT;
  }
  if (orientation_sensor_->sensor_interface_->IsDataValid()) {
    notify();  // only pass on the data if it is valid
  }
}  // end Update()

/**
 * @brief Get the current sensor configuration and place it in a JSON
 * object that can then be stored in non-volatile memory.
 *
 * @param doc JSON object to contain the configuration parameters
 * to be updated.
 */
void OrientationValues::get_configuration(JsonObject& doc) {
  doc["read_delay"] = read_delay_ms_;
}  // end get_configuration()

/**
 * @brief Fetch the JSON format used for holding the configuration.
 */
String OrientationValues::get_config_schema() { return FPSTR(SCHEMA); }

/**
 * @brief Use the values stored in JSON object config to update
 * the appropriate member variables.
 *
 * @param config JSON object containing the configuration parameters
 * to be updated.
 * @return True if successful; False if a parameter could not be found.
 */
bool OrientationValues::set_configuration(const JsonObject& config) {
  String expected[] = {"read_delay"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  read_delay_ms_ = config["read_delay"];
  return true;
}
