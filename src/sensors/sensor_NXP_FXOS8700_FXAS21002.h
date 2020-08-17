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

// Orientation data are intended as a SignalK data source, but can also
//   be viewed on the serial port stream, or using the OrientationVisualiser
//   example in the Adafruit AHRS Processing library.

// Based on  https://github.com/PaulStoffregen/NXPMotionSense with adjustments
//   to Adafruit Unified Sensor interface.
//   PJRC & Adafruit invest time and resources providing this open source code,
//   please support PJRC and open-source hardware by purchasing products
//   from PJRC!  Written by PJRC, adapted by Limor Fried for Adafruit Industries.

// Last edits by Bjarne Hansen 2020-08-16

// Relies on Serial object existing for sending data/diagnostics to serial port

#include <stdint.h>

#include <Adafruit_AHRS.h>
#include "Adafruit_Sensor_Calibration.h"

class SensorNXP_FXOS8700_FXAS21002 {

 public:
  SensorNXP_FXOS8700_FXAS21002(void);
  bool connect(uint8_t pin_i2c_sda, uint8_t pin_i2c_scl);
  void printSensorDetails(void);
  void initFilter(int sampling_interval_ms);
  float getHeadingDegrees(void);
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

  Adafruit_Sensor *accelerometer, *gyroscope, *magnetometer;
  Adafruit_Sensor_Calibration_EEPROM  cal; // use EEPROM on ESP32 module
  byte caldata[68];  // buffer to receive magnetic calibration data
  byte calcount = 0;
  bool isCalibrated = false;
  sensors_event_t mag_event, gyro_event, accel_event;
  uint32_t timestamp;
  uint32_t last_print_time = 0;
  float roll, pitch, heading;  // rotation values calculated by filter function,
                               // and reported to outside world
  float gx, gy, gz;  // angular velocity (gyroscope) calculated by filter
                     // function, and reported to outside world
  // linear acceleration readings are stored in accel_event

  bool initSensors(void);
  void setupSensors(void);
  void receiveCalibration(void);
  uint16_t crc16_update(uint16_t crc, uint8_t a);

};

#endif  //_SENSORS_NXP_FXOS8700_FXAS21002_H