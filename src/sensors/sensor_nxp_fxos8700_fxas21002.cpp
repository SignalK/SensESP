#include "sensor_nxp_fxos8700_fxas21002.h"

#include <RemoteDebug.h>

#include "sensesp.h"

//#define AHRS_DEBUG_OUTPUT  //output diagnostics from sensor read and filter
#define MIN_PRINT_INTERVAL_MS \
  (200)  // when result printing is requested, it won't happen
         // any more frequently than this,
         // to avoid overloading the serial channel
#define FXAS_CR1_ACTIVE_BITMASK (0b00000010) //Active bit in gyro CTRL_REG1
#define FXAS_CR1_READY_BITMASK (0b00000001) //Ready bit in gyro CTRL_REG1
#define FXAS_CR1_ODR_BITMASK (0b00011100) //Output Data Rate in gyro CTRL_REG1
#define FXAS_CR1_ODR_BITS_LOC (2) //ODR bits are in positions 4:2
#define N2K_INVALID_FLOAT (-1e-9) //NMEA2000 value for unavailable parameters

Adafruit_NXPSensorFusion filter;  // when placed inside class, results from this
                                  // filter return Heading=0.0 always. Other two
                                  // filters (see below) work OK inside class

//  Constructor creates an accelerometer/magnetometer object (fxos_)
//  and a gyroscope object (fxas_)
SensorNXP_FXOS8700_FXAS21002::SensorNXP_FXOS8700_FXAS21002()
    : fxos_(0x8700A, 0x8700B), fxas_(0x0021002C) {}

//  Connect to FXOS8700 & FXAS21002 sensor combination using I2C.
//  To use default Arduino I2C pins, pass pin_i2c_sda and pin_i2c_scl = -1
//  Load calibration values if available (readings won't be valid without
//    calibration).
//  Ranges/sensitivity use ICs' default values, then setupSensors() adjusts.
bool SensorNXP_FXOS8700_FXAS21002::connect(int pin_i2c_sda,
                                           int pin_i2c_scl) {
  debugI("NXP 9 Degrees-of-Freedom Sensor with Adafruit AHRS");
  if (!cal_.begin()) {
    debugE("Failed to initialize calibration helper");
    is_calibrated_ = false; 
  }else { 
    if (!cal_.loadCalibration()) {
      debugI("No calibration loaded/found...will start with defaults");
    is_calibrated_ = false;
    } else {
      debugI("Loaded existing calibration:");
    cal_.printSavedCalibration();
    is_calibrated_ = true;
    }
  }//end of loading calibration

  Wire.begin(pin_i2c_sda, pin_i2c_scl, 400000 );
  if (!startSensors()) {
      debugE("Failed to find sensors");
    return false;
  }

  setupSensors();       //set ranges, etc

  return true;
}  // end connect()

//  Prints details of FXOS8700 & FXAS21002 sensor combination
void SensorNXP_FXOS8700_FXAS21002::printSensorDetails(void) {
  accelerometer_->printSensorDetails();
  gyroscope_->printSensorDetails();
  magnetometer_->printSensorDetails();
}  // end printSensorDetails()

void SensorNXP_FXOS8700_FXAS21002::initFilter(int sampling_interval_ms) {
  roll_ = 0.0;
  pitch_ = 0.0;
  heading_ = 0.0;
  gx_ = 0.0;
  gy_ = 0.0;
  gz_ = 0.0;
  sampling_rate_hz_ = 1000.0 / sampling_interval_ms;
  filter.begin(sampling_rate_hz_);
  timestamp_ = millis();
}  // end initFilter()

/* The following get___() methods return the specified parameter
of the nine measured parameters. Terminology is nautical
(e.g. heading, pitch, roll) and assumes the physical sensor
is aligned with the X-axis pointing to the Bow;
the Y-axis pointing to Port; and the Z-axis pointing up. If your
sensor is mounted differently, changes to the terminology/axes
will need to be made.
Per Signal K convention, units are SI:  radians for headings;
radians / second for turn rates; and meters / second^2 for acceleration.
*/
float SensorNXP_FXOS8700_FXAS21002::getHeadingRadians(void) {
  return (360.0 - heading_) / SENSORS_RADS_TO_DPS;
}  // end getHeadingRadians()

float SensorNXP_FXOS8700_FXAS21002::getPitchRadians(void) {
  return (-pitch_) / SENSORS_RADS_TO_DPS;
}  // end getPitchRadians()

float SensorNXP_FXOS8700_FXAS21002::getRollRadians(void) {
  return roll_ / SENSORS_RADS_TO_DPS;
}  // end getRollRadians()

float SensorNXP_FXOS8700_FXAS21002::getAccelerationX(void) {
  return accel_event_.acceleration.x;
}  // end getAccelerationX()

float SensorNXP_FXOS8700_FXAS21002::getAccelerationY(void) {
  return accel_event_.acceleration.y;
}  // end getAccelerationY()

float SensorNXP_FXOS8700_FXAS21002::getAccelerationZ(void) {
  return accel_event_.acceleration.z;
}  // end getAccelerationZ()

float SensorNXP_FXOS8700_FXAS21002::getRateOfTurn(void) {
  return (-gz_) / SENSORS_RADS_TO_DPS;
}  // end getRateOfTurn()

float SensorNXP_FXOS8700_FXAS21002::getRateOfPitch(void) {
  return (-gy_) / SENSORS_RADS_TO_DPS;
}  // end getRateOfPitch()

float SensorNXP_FXOS8700_FXAS21002::getRateOfRoll(void) {
  return gx_ / SENSORS_RADS_TO_DPS;
}  // end getRateOfRoll()

// fetches data from sensors, applies filter function, and assigns orientation
// data to member variables as pitch/roll/heading and quaternion.
void SensorNXP_FXOS8700_FXAS21002::gatherOrientationDataOnce(
    bool is_print_results) {

  timestamp_ = millis();

  //confirm we are connected to sensors, attempt reconnect if not
  if( !startSensors() ) {  
    debugW("No connection with orientation sensors!");
    //assign orientation value that in NMEA2000 means invalid/unavailable
    gx_ = N2K_INVALID_FLOAT;
    gy_ = N2K_INVALID_FLOAT;
    gz_ = N2K_INVALID_FLOAT;
    roll_ = N2K_INVALID_FLOAT;
    pitch_ = N2K_INVALID_FLOAT;
    heading_ = N2K_INVALID_FLOAT;
    accel_event_.acceleration.x = N2K_INVALID_FLOAT;
    accel_event_.acceleration.y = N2K_INVALID_FLOAT;
    accel_event_.acceleration.z = N2K_INVALID_FLOAT;
    return;
  }

  if (!is_calibrated_) {
    debugW("Orientation is uncalibrated!");
  }
  magnetometer_->getEvent(&mag_event_);
  gyroscope_->getEvent(&gyro_event_);
  accelerometer_->getEvent(&accel_event_);

#if defined(AHRS_DEBUG_OUTPUT)
  debugI("I2C took %lu ms", millis() - timestamp);
#endif

  if( !cal_.calibrate(mag_event_) ) {
    debugW("Could not calibrate mag!");
  }
  if( !cal_.calibrate(accel_event_) ) {
    debugW("Could not calibrate accel!");
  }
  if( !cal_.calibrate(gyro_event_) ) {
    debugW("Could not calibrate gyro!");
  }
  // Gyroscope needs to be converted from Rad/s to Degree/s for filter
  // the rest are not unit-important
  gx_ = gyro_event_.gyro.x * SENSORS_RADS_TO_DPS;
  gy_ = gyro_event_.gyro.y * SENSORS_RADS_TO_DPS;
  gz_ = gyro_event_.gyro.z * SENSORS_RADS_TO_DPS;

  // Apply the desired filter
  filter.update(gx_, gy_, gz_, 
                accel_event_.acceleration.x,
                accel_event_.acceleration.y, 
                accel_event_.acceleration.z,
                mag_event_.magnetic.x, 
                mag_event_.magnetic.y,
                mag_event_.magnetic.z);
  roll_ = filter.getRoll();
  pitch_ = filter.getPitch();
  heading_ = filter.getYaw();
  float qw, qx, qy, qz;
  filter.getQuaternion(&qw, &qx, &qy, &qz);
  
#if defined(AHRS_DEBUG_OUTPUT)
  debugI("Update took %lu ms", millis() - timestamp);
  debugI("Raw: %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f",
          accel_event_.acceleration.x,
          accel_event_.acceleration.y,
          accel_event_.acceleration.z,
          gx, gy, gz,
          mag_event_.magnetic.x,
          mag_event_.magnetic.y,
          mag_event_.magnetic.z );
  debugI("Heading: %.1f", heading);
#endif

  if (is_print_results &&
      (millis() - last_print_time_ > MIN_PRINT_INTERVAL_MS)) {
    // print the heading, pitch and roll
    debugI("Orientation: %.2f, %.2f, %.2f", 
            heading_, pitch_, roll_);
    debugI("Quaternion: %.4f, %.4f, %.4f, %.4f", 
            qw, qx, qy, qz);
    last_print_time_ = millis();
  }

#if defined(AHRS_DEBUG_OUTPUT)
  debugI("Orientation collection + processing took %lu ms", millis() - timestamp);
#endif

}  // end gatherOrientationDataOnce()

// Fetches data from sensors and outputs to serial port in 
//  format that can be read by calibration utility/app. 
// Monitors same serial port for incoming calibration values 
//  sent by the utility, and writes them to EEPROM/Flash if
// received.
void SensorNXP_FXOS8700_FXAS21002::gatherCalibrationDataOnce(
    bool is_print_results) {  

  magnetometer_->getEvent(&mag_event_);
  gyroscope_->getEvent(&gyro_event_);
  accelerometer_->getEvent(&accel_event_);

  if (is_print_results) {
    // 'Raw' values in format matching expectation of MotionCal utility
    // This section copied verbatim from  Adafruit_AHRS_calibration.ino
    Serial.print(int(millis()));
    Serial.print(
        "Raw:");
    Serial.print(int(accel_event_.acceleration.x * 8192 / 9.8));
    Serial.print(",");
    Serial.print(int(accel_event_.acceleration.y * 8192 / 9.8));
    Serial.print(",");
    Serial.print(int(accel_event_.acceleration.z * 8192 / 9.8));
    Serial.print(",");
    Serial.print(int(gyro_event_.gyro.x * SENSORS_RADS_TO_DPS * 16));
    Serial.print(",");
    Serial.print(int(gyro_event_.gyro.y * SENSORS_RADS_TO_DPS * 16));
    Serial.print(",");
    Serial.print(int(gyro_event_.gyro.z * SENSORS_RADS_TO_DPS * 16));
    Serial.print(",");
    Serial.print(int(mag_event_.magnetic.x * 10));
    Serial.print(",");
    Serial.print(int(mag_event_.magnetic.y * 10));
    Serial.print(",");
    Serial.print(int(mag_event_.magnetic.z * 10));
    Serial.println("");

    // unified data
    Serial.print("Uni:");
    Serial.print(accel_event_.acceleration.x);
    Serial.print(",");
    Serial.print(accel_event_.acceleration.y);
    Serial.print(",");
    Serial.print(accel_event_.acceleration.z);
    Serial.print(",");
    Serial.print(gyro_event_.gyro.x, 4);
    Serial.print(",");
    Serial.print(gyro_event_.gyro.y, 4);
    Serial.print(",");
    Serial.print(gyro_event_.gyro.z, 4);
    Serial.print(",");
    Serial.print(mag_event_.magnetic.x);
    Serial.print(",");
    Serial.print(mag_event_.magnetic.y);
    Serial.print(",");
    Serial.print(mag_event_.magnetic.z);
    Serial.println("");
  }

  receiveCalibration(); //check for incoming cal values

  if (is_print_results) {
    Serial.print("Cal1:");
    for (int i = 0; i < 3; i++) {
      Serial.print(cal_.accel_zerog[i], 3);
      Serial.print(",");
    }
    for (int i = 0; i < 3; i++) {
      Serial.print(cal_.gyro_zerorate[i], 3);
      Serial.print(",");
    }
    for (int i = 0; i < 3; i++) {
      Serial.print(cal_.mag_hardiron[i], 3);
      Serial.print(",");
    }
    Serial.println(cal_.mag_field, 3);

    Serial.print("Cal2:");
    for (int i = 0; i < 9; i++) {
      Serial.print(cal_.mag_softiron[i], 4);
      if (i < 8) Serial.print(',');
    }
    Serial.println();
  }
}  // end gatherCalibrationDataOnce()


//If not already connected to sensors,
//try to connect and configure them.
//If already connected, check connection and
// return true if still responding.
bool SensorNXP_FXOS8700_FXAS21002::startSensors() {

  if( !is_connected_ ) {     
    if( fxos_.begin() && fxas_.begin() ) {
      //the Adafruit begin() routines setup the  sensors with these defaults:
      //  gyro: 100 Hz data rate, +/-250 dps full-scale, HPF off, active state, no FIFO,
      //  default LPF is 32Hz for ODR=100Hz
      is_connected_ = true;
      accelerometer_ = fxos_.getAccelerometerSensor();
      magnetometer_ = fxos_.getMagnetometerSensor();
      gyroscope_ = &fxas_;
      setupSensors();
    }
  }else { //already connected, we believe
    //ping each sensor to ensure it's still working/attached
    if( !pingFXAS21002() || !pingFXAS8700() ) {
      is_connected_ = false;  //ping unsuccessful: assume no connection
    }
  }

  return is_connected_;
}  // end startSensors()

void SensorNXP_FXOS8700_FXAS21002::setupSensors(void) {
  //Sets ranges for sensors, when defaults are not optimal.
  //Note that if the ranges are changed, the scaling of the output values
  //may need adjusting. For example, the accelerometer is operating
  //by default at +/-2g sensitivity (= 0.244 mg/LSB), and the accelerations
  //are converted to m/s^2. The conversion factor needs to scale 
  //proportionally to the sensitivity.
  //Register locations and settings are from the datasheets for the
  //FXAS21002 and FXOS8700:
  //https://www.nxp.com/docs/en/data-sheet/FXAS21002.pdf
  //https://www.nxp.com/docs/en/data-sheet/FXOS8700CQ.pdf

  //set gyro data rate to correspond with sampling rate, so 
  //internal LPF (low pass filter) will be suitable freq
  uint8_t odr_bits;
  if (sampling_rate_hz_ > 400.0) {
    odr_bits = 0b000;
  }else if(sampling_rate_hz_ > 200.0) {
    odr_bits = 0b001;
  }else if(sampling_rate_hz_ > 100.0) {
    odr_bits = 0b010;
  }else if(sampling_rate_hz_ > 50.0) {
    odr_bits = 0b011;
  }else if(sampling_rate_hz_ > 25.0) {
    odr_bits = 0b100;
  }else if(sampling_rate_hz_ > 12.5) {
    odr_bits = 0b101;
  }else {
    odr_bits = 0b110;
  }
  odr_bits = odr_bits << FXAS_CR1_ODR_BITS_LOC;  //shift left to bit posns 4:2

  byte ctl_reg1 = readByte(FXAS21002C_ADDRESS,GYRO_REGISTER_CTRL_REG1); //get reg value
  ctl_reg1 &= ~(FXAS_CR1_ACTIVE_BITMASK | FXAS_CR1_READY_BITMASK); //zero out the Active & Ready bits
  writeByte(FXAS21002C_ADDRESS,GYRO_REGISTER_CTRL_REG1, ctl_reg1); //put gyro in standby 
  ctl_reg1 = (ctl_reg1 & (~(FXAS_CR1_ODR_BITMASK | FXAS_CR1_ACTIVE_BITMASK))) | odr_bits | FXAS_CR1_ACTIVE_BITMASK;
  writeByte(FXAS21002C_ADDRESS,GYRO_REGISTER_CTRL_REG1, ctl_reg1 ); //back to Active mode

}  // end setupSensors()

//checks I2C bus to see whether FXAS21002 is connected and responding
bool SensorNXP_FXOS8700_FXAS21002::pingFXAS21002( void ) {
  if( FXAS21002C_ID == readByte(FXAS21002C_ADDRESS,GYRO_REGISTER_WHO_AM_I) ) {
    return true;
  } else {
    return false;
  }
} // end pingFXAS21002()


//checks I2C bus to see whether FXOS8700 is connected and responding
bool SensorNXP_FXOS8700_FXAS21002::pingFXAS8700( void ) {
  if( FXOS8700_ID == readByte(FXOS8700_ADDRESS,FXOS8700_REGISTER_WHO_AM_I) ) {
    return true;
  } else {
    return false;
  }
} // end pingFXAS21002()


//    Send value to reg register at address using Wire library
void SensorNXP_FXOS8700_FXAS21002::writeByte(byte address, byte reg, byte value) {
  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
} // end writeByte()

//  Read byte from reg register at address using Wire library
//  Returns 0 if error occurs on read.
byte SensorNXP_FXOS8700_FXAS21002::readByte(byte address,byte reg) {

  Wire.beginTransmission(address);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) {
    return 0;
  }
  Wire.requestFrom(address, (byte)1);
  return Wire.read();

} // end readByte()


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
    if (cal_count_ == 0 && b != 117) {
      // first byte must be 117
      return;
    }
    if (cal_count_ == 1 && b != 84) {
      // second byte must be 84
      cal_count_ = 0;
      return;
    }
    // store this byte
    cal_data_[cal_count_++] = b;
    if (cal_count_ < 68) {
      // full calibration message is 68 bytes
      return;
    }
    // verify the crc16 check
    crc = 0xFFFF;
    for (i = 0; i < 68; i++) {
      crc = crc16_update(crc, cal_data_[i]);
    }
    if (crc == 0) {
      // data looks good, use it
      float offsets[16];
      memcpy(offsets, cal_data_ + 2, 16 * 4);
      cal_.accel_zerog[0] = offsets[0];
      cal_.accel_zerog[1] = offsets[1];
      cal_.accel_zerog[2] = offsets[2];

      cal_.gyro_zerorate[0] = offsets[3];
      cal_.gyro_zerorate[1] = offsets[4];
      cal_.gyro_zerorate[2] = offsets[5];

      cal_.mag_hardiron[0] = offsets[6];
      cal_.mag_hardiron[1] = offsets[7];
      cal_.mag_hardiron[2] = offsets[8];

      cal_.mag_field = offsets[9];

      cal_.mag_softiron[0] = offsets[10];
      cal_.mag_softiron[1] = offsets[13];
      cal_.mag_softiron[2] = offsets[14];
      cal_.mag_softiron[3] = offsets[13];
      cal_.mag_softiron[4] = offsets[11];
      cal_.mag_softiron[5] = offsets[15];
      cal_.mag_softiron[6] = offsets[14];
      cal_.mag_softiron[7] = offsets[15];
      cal_.mag_softiron[8] = offsets[12];

      if (!cal_.saveCalibration()) {
        debugW("Couldn't save calibration");
      } else {
        debugI("Wrote calibration");
      }
      cal_.printSavedCalibration();
      cal_count_ = 0;
      return;
    }
    // look for the 117,84 in the data, before discarding
    for (i = 2; i < 67; i++) {
      if (cal_data_[i] == 117 && cal_data_[i + 1] == 84) {
        // found possible start within data
        cal_count_ = 68 - i;
        memmove(cal_data_, cal_data_ + i, cal_count_);
        return;
      }
    }
    // look for 117 in last byte
    if (cal_data_[67] == 117) {
      cal_data_[0] = 117;
      cal_count_ = 1;
    } else {
      cal_count_ = 0;
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