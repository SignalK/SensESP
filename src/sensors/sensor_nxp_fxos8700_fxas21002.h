#ifndef _SENSORS_NXP_FXOS8700_FXAS21002_H
#define _SENSORS_NXP_FXOS8700_FXAS21002_H

// Based on examples in the Adafruit AHRS library (Adafruit_AHRS_calibration.ino,
//    calibrated_orientation.ino).
// Connects to a supported magnetometer/gyro/accelerometer and outputs
//   orientation data as heading/pitch/roll, and also in quaternion form.

// Two data-reporting methods are available: one for
//   generating raw data used when calibrating the sensor; the
//   second for generating orientation data following calibration.

// For calibration, the sensor readings are output in PJRC Motion Sensor
//   Calibration Tool-compatible serial format. An external tool/app/program
//   is needed to process the raw serial data and generate calibration values.
//   Once generated, calibration values are stored in EEPROM and should not
//   need re-creation unless the magnetic environment (mounting location,
//   mounting enclosure, nearby magnetically-permeable objects) of the sensor
//   changes. Orientation data will not be reliable without prior calibration.

// Orientation data are intended as a Signal K data source, but can also
//   be viewed on the serial port stream, or using the OrientationVisualiser
//   example in the Adafruit AHRS Processing library.

// Based on  https://github.com/PaulStoffregen/NXPMotionSense with adjustments
//   to Adafruit Unified Sensor interface.
//   PJRC & Adafruit invest time and resources providing this open source code,
//   please support PJRC and open-source hardware by purchasing products
//   from PJRC!  Written by PJRC, adapted by Limor Fried for Adafruit Industries.

// Last edits by Bjarne Hansen 2020-10-20

// Relies on Serial object existing for sending data/diagnostics to serial port

#include <stdint.h>

#include <Adafruit_AHRS.h>
#include <Adafruit_FXAS21002C.h>
#include <Adafruit_FXOS8700.h>
#include <Adafruit_Sensor_Calibration.h>

class SensorNXP_FXOS8700_FXAS21002 {

 public:
  SensorNXP_FXOS8700_FXAS21002(void);
  bool connect(int pin_i2c_sda, int pin_i2c_scl);
  void printSensorDetails(void);
  void initFilter(int sampling_interval_ms);
  float getHeadingRadians(void);
  float getPitchRadians(void);
  float getRollRadians(void);
  float getAccelerationX(void);
  float getAccelerationY(void);
  float getAccelerationZ(void);
  float getRateOfTurn(void);
  float getRateOfPitch(void);
  float getRateOfRoll(void);
  void gatherCalibrationDataOnce(bool is_print_results);
  void gatherOrientationDataOnce(bool is_print_results);

 private:
  // pick one of three following filters: slower == better quality output
  // Adafruit_NXPSensorFusion filter; // slowest.  Note that this one needs to
  // be declared outside of class, else heading returns 0.0 always. Unsure
  // why. Currently declared in *.cpp file  The other two filters are fine when
  // declared inside class. Adafruit_NXP_SensorFusion filter runs fine on ESP32.
  // Adafruit_Madgwick filter;  // faster than NXP
  // Adafruit_Mahony filter;    // fastest/smallest

  Adafruit_FXOS8700 fxos_;    // the combined magnetometer + accelerometer
  Adafruit_FXAS21002C fxas_;  // the gyroscope
  Adafruit_Sensor *accelerometer_,  // accesses the accelerometer
                  *gyroscope_,      // accesses the gyro
                  *magnetometer_;   // accesses the magnetometer
  Adafruit_Sensor_Calibration_EEPROM cal_;  // use EEPROM on ESP32 module
  byte cal_data_[68];  // buffer to receive magnetic calibration data
  byte cal_count_ = 0;
  bool is_calibrated_ = false;
  bool is_connected_ = false; //indicates successful connection to sensor
  sensors_event_t mag_event_, gyro_event_, accel_event_;  // sensor readings
  uint32_t timestamp_;
  uint32_t last_print_time_ = 0;
  float roll_, pitch_, heading_;  // rotation values calculated by filter function,
                               // and reported to outside world
  float gx_, gy_, gz_;  // angular velocity (gyroscope) calculated by filter
                     // function, and reported to outside world
  // linear acceleration readings are stored in accel_event
  float sampling_rate_hz_ = 100.0; //used in setting filter, response rate of gyro, etc.

  bool startSensors(void);
  void setupSensors(void);
  void receiveCalibration(void);
  uint16_t crc16_update(uint16_t crc, uint8_t a);
  bool pingFXAS21002(void);
  bool pingFXAS8700(void);
  void writeByte(byte address, byte reg, byte value);
  byte readByte(byte address, byte reg);
};

#endif  //_SENSORS_NXP_FXOS8700_FXAS21002_H