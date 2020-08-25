#include "sensor_NXP_FXOS8700_FXAS21002.h"

#include <RemoteDebug.h>
#include "sensesp.h"
#include <Adafruit_FXAS21002C.h>
#include <Adafruit_FXOS8700.h>

//#define AHRS_DEBUG_OUTPUT  //output diagnostics from sensor read and filter
#define MIN_PRINT_INTERVAL_MS \
  (200)  // when result printing is requested, it won't happen
         // any more frequently than this,
         // to avoid overloading the serial channel

// can the following be moved inside class? or into main.cpp?
Adafruit_FXOS8700 fxos = Adafruit_FXOS8700(0x8700A, 0x8700B);  
Adafruit_FXAS21002C fxas = Adafruit_FXAS21002C(0x0021002C);

Adafruit_NXPSensorFusion filter;  // when placed inside class, results from this
                                  // filter return Heading=0.0 always. Other two
                                  // filters (see below) work OK inside class

SensorNXP_FXOS8700_FXAS21002::SensorNXP_FXOS8700_FXAS21002() {}

//  Connect to FXOS8700 & FXAS21002 sensor combination using I2C.
//  To use default Arduino I2C pins, pass pin_i2c_sda and pin_i2c_scl = -1
//  Load calibration values if available (readings won't be valid without
//    calibration).
//  TODO: set ranges/sensitivity. For now use ICs' default values.
bool SensorNXP_FXOS8700_FXAS21002::connect(uint8_t pin_i2c_sda,
                                           uint8_t pin_i2c_scl) {
  debugI("NXP 9 Degrees-of-Freedom Sensor with Adafruit AHRS");
  if (!cal.begin()) {
      debugE("Failed to initialize calibration helper");
    while (1) yield();
  }
  if (!cal.loadCalibration()) {
      debugI("No calibration loaded/found...will start with defaults");
    isCalibrated = false;
  } else {
      debugI("Loaded existing calibration:");
    cal.printSavedCalibration();
    isCalibrated = true;
  }

  Wire.begin(pin_i2c_sda, pin_i2c_scl);
  Wire.setClock(400000);  // 400KHz I2C (used to be after setupo())

  if (!initSensors()) {
      debugE("Failed to find sensors");
    return false;
  }

  setupSensors();       // TODO - can set ranges, etc
  printSensorDetails();

  return true;
}  // end connect()

//  Prints details of FXOS8700 & FXAS21002 sensor combination
void SensorNXP_FXOS8700_FXAS21002::printSensorDetails(void) {
  accelerometer->printSensorDetails();
  gyroscope->printSensorDetails();
  magnetometer->printSensorDetails();
}  // end printSensorDetails()

void SensorNXP_FXOS8700_FXAS21002::initFilter(int sampling_interval_ms) {
  roll = 0.0;
  pitch = 0.0;
  heading = 0.0;
  gx = 0.0;
  gy = 0.0;
  gz = 0.0;
  filter.begin(1000.0 / sampling_interval_ms);
  timestamp = millis();
}  // end initFilter()

float SensorNXP_FXOS8700_FXAS21002::getHeadingDegrees(void) {
  return heading;
}  // end getHeadingDegrees()

// fetches data from sensors, applies filter function, and assigns orientation
// data to member variables as pitch/roll/heading and quaternion.
void SensorNXP_FXOS8700_FXAS21002::gatherOrientationDataOnce(
    bool is_print_results) {

  timestamp = millis();

  if (!isCalibrated) {
    debugW("Orientation is uncalibrated!");
  }
  magnetometer->getEvent(&mag_event);
  gyroscope->getEvent(&gyro_event);
  accelerometer->getEvent(&accel_event);

#if defined(AHRS_DEBUG_OUTPUT)
  DebugI("I2C took %d ms", millis() - timestamp);
#endif

  cal.calibrate(mag_event);
  cal.calibrate(accel_event);
  cal.calibrate(gyro_event);
  // Gyroscope needs to be converted from Rad/s to Degree/s for filter
  // the rest are not unit-important
  gx = gyro_event.gyro.x * SENSORS_RADS_TO_DPS;
  gy = gyro_event.gyro.y * SENSORS_RADS_TO_DPS;
  gz = gyro_event.gyro.z * SENSORS_RADS_TO_DPS;

  // Apply the desired filter
  filter.update(gx, gy, gz, 
                accel_event.acceleration.x,
                accel_event.acceleration.y, 
                accel_event.acceleration.z,
                mag_event.magnetic.x, 
                mag_event.magnetic.y,
                mag_event.magnetic.z);
  roll = filter.getRoll();
  pitch = filter.getPitch();
  heading = filter.getYaw();
  float qw, qx, qy, qz;
  filter.getQuaternion(&qw, &qx, &qy, &qz);

#if defined(AHRS_DEBUG_OUTPUT)
  debugI("Update took %d ms", millis() - timestamp);
  debugI("Raw: %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f",
          accel_event.acceleration.x,
          accel_event.acceleration.y,
          accel_event.acceleration.z,
          gx, gy, gz,
          mag_event.magnetic.x,
          mag_event.magnetic.y,
          mag_event.magnetic.z;
  debugI("Heading: %.1f", heading);
#endif

  if (is_print_results &&
      (millis() - last_print_time > MIN_PRINT_INTERVAL_MS)) {
    // print the heading, pitch and roll
    debugI("Orientation: %.2f, %.2f, %.2f", 
            heading, pitch, roll);
    debugI("Quaternion: %.4f, %.4f, %.4f, %.4f", 
            qw, qx, qy, qz);
    last_print_time = millis();
  }
#if defined(AHRS_DEBUG_OUTPUT)
  DebugI("I2C took %d ms", millis() - timestamp);
#endif
}  // end gatherOrientationDataOnce()

// Fetches data from sensors and outputs to serial port in 
//  format that can be read by calibration utility/app. 
// Monitors same serial port for incoming calibration values 
//  sent by the utility, and writes them to EEPROM/Flash if
// received.
void SensorNXP_FXOS8700_FXAS21002::gatherCalibrationDataOnce(
    bool is_print_results) {  

  magnetometer->getEvent(&mag_event);
  gyroscope->getEvent(&gyro_event);
  accelerometer->getEvent(&accel_event);

  if (is_print_results) {
    // 'Raw' values in format matching expectation of MotionCal utility
    // This section copied verbatim from  Adafruit_AHRS_calibration.ino
    Serial.print(int(millis()));
    Serial.print(
        "Raw:");
    Serial.print(int(accel_event.acceleration.x * 8192 / 9.8));
    Serial.print(",");
    Serial.print(int(accel_event.acceleration.y * 8192 / 9.8));
    Serial.print(",");
    Serial.print(int(accel_event.acceleration.z * 8192 / 9.8));
    Serial.print(",");
    Serial.print(int(gyro_event.gyro.x * SENSORS_RADS_TO_DPS * 16));
    Serial.print(",");
    Serial.print(int(gyro_event.gyro.y * SENSORS_RADS_TO_DPS * 16));
    Serial.print(",");
    Serial.print(int(gyro_event.gyro.z * SENSORS_RADS_TO_DPS * 16));
    Serial.print(",");
    Serial.print(int(mag_event.magnetic.x * 10));
    Serial.print(",");
    Serial.print(int(mag_event.magnetic.y * 10));
    Serial.print(",");
    Serial.print(int(mag_event.magnetic.z * 10));
    Serial.println("");

    // unified data
    Serial.print("Uni:");
    Serial.print(accel_event.acceleration.x);
    Serial.print(",");
    Serial.print(accel_event.acceleration.y);
    Serial.print(",");
    Serial.print(accel_event.acceleration.z);
    Serial.print(",");
    Serial.print(gyro_event.gyro.x, 4);
    Serial.print(",");
    Serial.print(gyro_event.gyro.y, 4);
    Serial.print(",");
    Serial.print(gyro_event.gyro.z, 4);
    Serial.print(",");
    Serial.print(mag_event.magnetic.x);
    Serial.print(",");
    Serial.print(mag_event.magnetic.y);
    Serial.print(",");
    Serial.print(mag_event.magnetic.z);
    Serial.println("");
  }

  receiveCalibration(); //check for incoming cal values

  if (is_print_results) {
    Serial.print("Cal1:");
    for (int i = 0; i < 3; i++) {
      Serial.print(cal.accel_zerog[i], 3);
      Serial.print(",");
    }
    for (int i = 0; i < 3; i++) {
      Serial.print(cal.gyro_zerorate[i], 3);
      Serial.print(",");
    }
    for (int i = 0; i < 3; i++) {
      Serial.print(cal.mag_hardiron[i], 3);
      Serial.print(",");
    }
    Serial.println(cal.mag_field, 3);

    Serial.print("Cal2:");
    for (int i = 0; i < 9; i++) {
      Serial.print(cal.mag_softiron[i], 4);
      if (i < 8) Serial.print(',');
    }
    Serial.println();
  }
}  // end gatherCalibrationDataOnce()


bool SensorNXP_FXOS8700_FXAS21002::initSensors() {
  if (!fxos.begin() || !fxas.begin()) {
    return false;
  }
  accelerometer = fxos.getAccelerometerSensor();
  magnetometer = fxos.getMagnetometerSensor();
  gyroscope = &fxas;

  return true;
}  // end initSensors()

void SensorNXP_FXOS8700_FXAS21002::setupSensors(void) {
  // we could set the g range for accelerometer here, for example
}  // end setupSensors()

// Receives calibration values sent by external calibration utility
//  over serial port. For details and format, see
// https://github.com/PaulStoffregen/MotionCal
void SensorNXP_FXOS8700_FXAS21002::receiveCalibration(void) {
  uint16_t crc;
  byte b, i;

  while (
      Serial.available()) {  // unsure why while() here in example - due to later
                             // returns, there is only ever 1 byte collected per
                             // invocation of receiveCalibration() unless fails
                             // crc and 117,84 not found in data - then the final
                             // if() is entered, and we read 2 bytes.
    b = Serial.read();
    if (calcount == 0 && b != 117) {
      // first byte must be 117
      return;
    }
    if (calcount == 1 && b != 84) {
      // second byte must be 84
      calcount = 0;
      return;
    }
    // store this byte
    caldata[calcount++] = b;
    if (calcount < 68) {
      // full calibration message is 68 bytes
      return;
    }
    // verify the crc16 check
    crc = 0xFFFF;
    for (i = 0; i < 68; i++) {
      crc = crc16_update(crc, caldata[i]);
    }
    if (crc == 0) {
      // data looks good, use it
      float offsets[16];
      memcpy(offsets, caldata + 2, 16 * 4);
      cal.accel_zerog[0] = offsets[0];
      cal.accel_zerog[1] = offsets[1];
      cal.accel_zerog[2] = offsets[2];

      cal.gyro_zerorate[0] = offsets[3];
      cal.gyro_zerorate[1] = offsets[4];
      cal.gyro_zerorate[2] = offsets[5];

      cal.mag_hardiron[0] = offsets[6];
      cal.mag_hardiron[1] = offsets[7];
      cal.mag_hardiron[2] = offsets[8];

      cal.mag_field = offsets[9];

      cal.mag_softiron[0] = offsets[10];
      cal.mag_softiron[1] = offsets[13];
      cal.mag_softiron[2] = offsets[14];
      cal.mag_softiron[3] = offsets[13];
      cal.mag_softiron[4] = offsets[11];
      cal.mag_softiron[5] = offsets[15];
      cal.mag_softiron[6] = offsets[14];
      cal.mag_softiron[7] = offsets[15];
      cal.mag_softiron[8] = offsets[12];

      if (!cal.saveCalibration()) {
        debugW("Couldn't save calibration");
      } else {
        debugI("Wrote calibration");
      }
      cal.printSavedCalibration();
      calcount = 0;
      return;
    }
    // look for the 117,84 in the data, before discarding
    for (i = 2; i < 67; i++) {
      if (caldata[i] == 117 && caldata[i + 1] == 84) {
        // found possible start within data
        calcount = 68 - i;
        memmove(caldata, caldata + i, calcount);
        return;
      }
    }
    // look for 117 in last byte
    if (caldata[67] == 117) {
      caldata[0] = 117;
      calcount = 1;
    } else {
      calcount = 0;
    }
  }
}  // end receiveCalibration()

uint16_t SensorNXP_FXOS8700_FXAS21002::crc16_update(uint16_t crc, uint8_t a) {
  // CRC implementation as expected by external calibration utility
  int i;
  crc ^= a;
  for (i = 0; i < 8; i++) {
    if (crc & 1) {
      crc = (crc >> 1) ^ 0xA001;
    } else {
      crc = (crc >> 1);
    }
  }
  return crc;
}  // end crc16_update()