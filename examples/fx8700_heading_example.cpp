#include <Arduino.h>
#include <Wire.h>

#include <sstream>
#include <string>

#include "sensesp_app.h"
#include "sensors/orientation_sensor.h"
#include "signalk/signalk_output.h"

// Sensor hardware details: I2C addresses and pins       
#define BOARD_ACCEL_MAG_I2C_ADDR    (0x1F) ///< I2C address on Adafruit breakout board
#define BOARD_GYRO_I2C_ADDR         (0x21) ///< I2C address on Adafruit breakout board
#if defined( ESP8266 )
  #define PIN_I2C_SDA (12)  //Adjust to your board. A value of -1
  #define PIN_I2C_SCL (14)  // will use default Arduino pins.
#elif defined( ESP32 )
  #define PIN_I2C_SDA (23)  //Adjust to your board. A value of -1
  #define PIN_I2C_SCL (25)  //will use default Arduino pins.
#endif

// How often orientation parameters are published via Signal K message
#define ORIENTATION_REPORTING_INTERVAL_MS (100)

// SensESP builds upon the ReactESP framework. Every ReactESP application
// defines an "app" object vs defining a "main()" method.
ReactESP app([]() {

// Some initialization boilerplate when in debug mode...
#ifndef SERIAL_DEBUG_DISABLED
  SetupSerialDebug(115200);
#endif

  /**
   * Create the global SensESPApp() object.
   * By passing the WiFi setup details in the constructor, rather than
   * relying on entering it in the device's web interface, we save about
   * 2496 bytes of heap memory (RAM). Another alternative is to use the
   * Builder pattern (sensesp_app_builder.h), but that saves only 1880 bytes.
   */
  sensesp_app = new SensESPApp(
      "SensESP_D1",         //hostname (name of this ESP device as advertised to SignalK)
      "mySSID",             //WiFi network SSID
      "myPassword",         //WiFi network password
      "192.168.1.4",        //IP address of network's Signal K server
      3000);                //port on which to connect to Signal K server

  /**
   * The "SignalK path" identifies this sensor to the Signal K server. Leaving
   * this blank would indicate this particular sensor or transform does not
   * broadcast Signal K data.
   * If you have multiple sensors connected to your microcontroller (ESP),
   * each of them will (probably) have its own Signal K path variable. For
   * example, if you have two propulsion engines, and you want the RPM of
   * each of them to go to Signal K, you might have
   * sk_path_portEngine = "propulsion.port.revolutions" and
   * sk_path_starboardEngine = "propulsion.starboard.revolutions"
   * To find valid Signal K Paths look at this link (or later version):
   * @see https://signalk.org/specification/1.5.0/doc/vesselsBranch.html
   *
   * Vessel heading can be indicated as headingCompass (uncorrected for
   * Deviation) or as part of an attitude data group (i.e. yaw, pitch, roll).
   * This example provides both.
   */
  const char* kSKPathHeading = "navigation.headingCompass";
  const char* kSKPathAttitude = "navigation.attitude";
  /**
   * This example shows heading, pitch, and roll. If you want other parameters
   * as well, uncomment the appropriate path(s) from the following.
   * Signal K v1.5 does not describe paths for roll rate and pitch rate
   * so these are provided using the same pattern as for rateOfTurn.
   * Signal K v1.5 says path for temperature can include zone.
   * Replace ecompass with a different zone if desired.
   * Signal K v1.5 does not describe a path for acceleration.
   */
  // const char* kSKPathTurnRate   = "navigation.rateOfTurn";
  // const char* kSKPathRollRate   = "navigation.rateOfRoll";
  // const char* kSKPathPitchRate  = "navigation.rateOfPitch";
  // const char* kSKPathTemperature =
  //                "environment.inside.ecompass.temperature"; 
  // const char* kSKPathAccel = "sensors.accelerometer.accel_xyz";

  /**
   * If you are creating a new Signal K path that does not
   * already exist in the specification, it is best to
   * define "metadata" that describes your new value. This
   * metadata will be reported to the Signal K server the first
   * time your sensor reports its value(s) to the server.
   */
  // Uncomment from the following as needed.
  //   SKMetadata* metadata_accel = new SKMetadata();
  //   metadata_accel->description_ = "Acceleration in X,Y,Z axes";
  //   metadata_accel->display_name_ = "Accelerometer";
  //   metadata_accel->short_name_ = "Accel";
  //   metadata_accel->units_ = "m/s^2";
  //
  //   SKMetadata* metadata_rate_of_roll = new SKMetadata();
  //   metadata_rate_of_roll->description_ =
  //        "Rate of Roll about bow-stern axis";
  //   metadata_rate_of_roll->display_name_ = "Roll Rate";
  //   metadata_rate_of_roll->short_name_ = "Roll Rate";
  //   metadata_rate_of_roll->units_ = "rad/s";
  //
  //   SKMetadata* metadata_rate_of_pitch = new SKMetadata();
  //   metadata_rate_of_pitch->description_ =
  //        "Rate of Pitch about port-starboard axis";
  //   metadata_rate_of_pitch->display_name_ = "Pitch Rate";
  //   metadata_rate_of_pitch->short_name_ = "Pitch Rate";
  //   metadata_rate_of_pitch->units_ = "rad/s";
  //
  //   SKMetadata* metadata_temperature = new SKMetadata();
  //   metadata_temperature->description_ =
  //        "Temperature reported by orientation sensor"
  //   metadata_temperature->display_name_ = "Temperature at eCompass";
  //   metadata_temperature->short_name_ = "Temp";
  //   metadata_temperature->units_ = "K";

  /**
   * The "Configuration path" is combined with "/config" to formulate a URL
   * used by the RESTful API for retrieving or setting configuration data.
   * It is ALSO used to specify a path to the file system
   * where configuration data is saved on the MCU board. It should
   * ALWAYS start with a forward slash if specified. If left blank,
   * that indicates this sensor or transform does not have any
   * configuration to save, or that you're not interested in doing
   * run-time configuration.
   * These two are necessary until a method is created to synthesize them.
   */
  const char* kConfigPathAttitude = "/sensors/attitude/value_settings";
  const char* kConfigPathAttitude_SK = "/sensors/attitude/sk";
  const char* kConfigPathHeading = "/sensors/heading/value_settings";
  const char* kConfigPathHeading_SK = "/sensors/heading/sk";
  /**
   Above arrangement of config paths yields this web interface structure:
   Note the hardware sensor itself has no run-time configurable items.
   sensors->attitude
                   ->sk_path
                   ->value_settings
           ->heading
                   ->sk_path
                   ->value_settings
  */
  // This example shows attitude and compass heading. If you want other parameters
  // as well, uncomment and modify the appropriate path(s) from the following 
  // or create new paths as needed.
  //   const char* kConfigPathTurnRate_SK    = "/sensors/rateOfTurn/sk_path";
  //   const char* kConfigPathTurnRate       = "/sensors/rateOfTurn/value_settings";
  //   const char* kConfigPathAccelXYZ       = "/sensors/acceleration/value_settings";
  //   const char* kConfigPathAccelXYZ_SK    = "/sensors/acceleration/sk_path";
  //   const char* kConfigPathTemperature    = "/sensors/temperature/value_settings";
  //   const char* kConfigPathTemperature_SK = "/sensors/temperature/sk_path";

  /**
   * Create and initialize the Orientation data source.
   * This uses a 9 Degrees-of-freedom combination sensor that provides multiple
   * orientation parameters. Selection of which particular parameters are
   * output is performed later when the value producers are created.
   */
  auto* orientation_sensor = new OrientationSensor(
      PIN_I2C_SDA, PIN_I2C_SCL, BOARD_ACCEL_MAG_I2C_ADDR, BOARD_GYRO_I2C_ADDR);

  /* Magnetic Calibration occurs during regular runtime. After power-on, move
   * the sensor through a series of rolls, pitches and yaws. After enough
   * readings have been collected (takes 15-30 seconds when rotating the sensor
   * by hand) then the sensor should be calibrated.
   * A Magnetic Calibration can be saved in non-volatile memory, so it will be
   * loaded at the next power-up. To save a calibration, use the
   * value_settings->Save_Mag_Cal entry in the sensor web interface.
   * A calibration will be valid until the sensor's magnetic environment
   * changes.
   * TODO: It is possible to have an indication that the sensor is uncalibrated
   * but this has not been implemented.
   */

  /*
   * Create the desired outputs from orientation sensor. Note that the physical
   * sensor is read at whatever rate is specified in the Sensor Fusion library's
   * build.h file (#define FUSION_HZ), currently set to 40 Hz. Fusion
   * calculations are run at that same rate. This is different than, and
   * usually faster than, the rate at which orientation parameters are output.
   * Reportng orientation values within SensESP can happen at any desired
   * rate, though if it is more often than the fusion rate then
   * there will be duplicated values. This example uses a 10 Hz outout rate.
   * It is not necessary that all the values be output at the same rate (for
   * example, it likely makes sense to report temperature at a slower rate).
   */
  auto* sensor_heading = new OrientationValues(
      orientation_sensor, OrientationValues::kCompassHeading,
      ORIENTATION_REPORTING_INTERVAL_MS, kConfigPathHeading);
  sensor_heading->connect_to(
      new SKOutputNumber(kSKPathHeading, kConfigPathHeading_SK));

  auto* sensor_attitude = new AttitudeValues(
      orientation_sensor, ORIENTATION_REPORTING_INTERVAL_MS,
      kConfigPathAttitude);
  sensor_attitude->connect_to(
      new SKOutputAttitude(kSKPathAttitude, kConfigPathAttitude_SK));

  // This example reports attitude and heading. If you want other parameters
  // as well, uncomment the appropriate connections from the following.
  //   auto* sensor_turn_rate = new OrientationValues(
  //       orientation_sensor, OrientationValues::kRateOfTurn,
  //       ORIENTATION_REPORTING_INTERVAL_MS, kConfigPathTurnRate);
  //   sensor_turn_rate->connect_to(
  //       new SKOutputNumber(kSKPathTurnRate, kConfigPathTurnRate_SK));

  //   auto* sensor_roll_rate = new OrientationValues(
  //       orientation_sensor, OrientationValues::kRateOfRoll,
  //       ORIENTATION_REPORTING_INTERVAL_MS, kConfigPathRollRate);
  //   sensor_roll_rate->connect_to(
  //       new SKOutputNumber(kSKPathRollRate, kConfigPathRollRate_SK));

  //   auto* sensor_pitch_rate = new OrientationValues(
  //       orientation_sensor, OrientationValues::kRateOfPitch,
  //       ORIENTATION_REPORTING_INTERVAL_MS, kConfigPathPitchRate);
  //   sensor_pitch_rate->connect_to(
  //       new SKOutputNumber(kSKPathPitchRate, kConfigPathPitchRate_SK));

  // TODO - it makes sense to send all three accel values (XYZ) in
  // one SK package. The needed data structure is not yet defined in
  // SensESP. The following sends the X accel as a single value.
  //   auto* sensor_accel_x = new OrientationValues(
  //       orientation_sensor, OrientationValues::kAccelerationX,
  //       ORIENTATION_REPORTING_INTERVAL_MS, kConfigPathAccelXYZ);
  //   sensor_accel_x->connect_to(
  //       new SKOutputNumber(kSKPathAccel, kConfigPathAccelXYZ_SK));

  //   auto* sensor_temperature =
  //       new OrientationValues(orientation_sensor,
  //       OrientationValues::kTemperature,
  //                             1000, kConfigPathTemperature);
  //   sensor_temperature
  //       ->connect_to(new Linear(1.0, 0.0, "/sensors/temperature/calibrate"))
  //       ->connect_to(new SKOutputNumber(
  //           kSKPathTemperature, kConfigPathTemperature_SK,
  //           metadata_temperature));

  /**
   *  Relationship of the Axes and the terminology:
   * X,Y,Z are orthogonal, in a Right-handed coordinate system:
   * think of the X axis pointing West, Y pointing South, and Z pointing up.
   * On the Adafruit FXOS8700/FXAS21002 sensor PCB, the axes are printed
   * on the top-side silkscreen.
   * 
   * Acceleration is measured in the direction of the corresponding axis.
   * 
   * If the sensor is mounted with the X-axis pointing to the bow of the boat
   * and the Y-axis pointing to Port, then Z points up and the following applies:
   *  Heading is rotation about the Z-axis. It increases with rotation to starboard.
   *  Pitch is rotation about the Y-axis. Positive is when the bow points up.
   *  Roll is rotation about the X-axis. Positive is rolling to starboard.
   *  Turn-rate is rotation about the Z-axis. Positive is increasing bearing.
   *  Roll-rate is rotation about the X-axis. Positive is rolling to starboard.
   *  Pitch-rate is rotation about the Y-axis. Positive is bow rising.
   * 
   * If the sensor is mounted differently, or you prefer an alternate nomenclature,
   * the get___() methods in sensor_fusion_class.cpp can be adjusted.
  */

  // Start the SensESP application running
  sensesp_app->enable();
});
