#include <Adafruit_Sensor.h>
#include <Arduino.h>
#include <Wire.h>

#include <sstream>
#include <string>

#include "sensesp_app.h"
#include "sensors/orientation_9dof_input.h"
#include "sensors/sensor_nxp_fxos8700_fxas21002.h"
#include "signalk/signalk_output.h"

#define PIN_I2C_SDA (23)  // Adjust to your board. A value of -1
#define PIN_I2C_SCL (25)  // will use default Arduino pins.

#define ORIENTATION_SAMPLING_INTERVAL_MS (100)

// SensESP builds upon the ReactESP framework. Every ReactESP application
// defines an "app" object vs defining a "main()" method.
ReactESP app([]() {

// Start serial debug if not disabled
#ifndef SERIAL_DEBUG_DISABLED
  SetupSerialDebug(115200);
#endif

  // Create the global SensESPApp() object.
  sensesp_app = new SensESPApp();

  /*
     The "SignalK path" identifies this sensor to the SignalK server. Leaving
     this blank would indicate this particular sensor (or transform) does not
     broadcast SignalK data.
     If you have multiple sensors connected to your microcontoller (ESP), each
     one of them will (probably) have its own SignalK path variable. For
     example, if you have two propulsion engines, and you want the RPM of each
     of them to go to SignalK, you might have sk_path_portEngine =
     "propulsion.port.revolutions" and sk_path_starboardEngine =
     "propulsion.starboard.revolutions"

     To find valid Signal K Paths that fits your need you look at this link:
     https://signalk.org/specification/1.4.0/doc/vesselsBranch.html#vesselsregexpelectrical
  */
  const char* sk_path_heading = "navigation.headingMagnetic";
  const char* sk_path_attitude = "navigation.attitude";
  const char* sk_path_rate_Of_turn = "navigation.rateOfTurn";

  /*
     This example shows heading, attitude (roll, pitch, yaw) and rate of turn.
     If you want other parameters as well, uncomment the appropriate path(s)
     from the following:
  */
  // TODO: Comment below lines before merge into master!
  const char* sk_path_accel_x = "FX8700.accel_x";
  const char* sk_path_accel_y = "FX8700.accel_y";
  const char* sk_path_accel_z = "FX8700.accel_z";
  const char* sk_path_roll_rate = "FX8700.roll_rate";
  const char* sk_path_pitch_rate = "FX8700.pitch_rate";

  /*
     The "Configuration path" is combined with "/config" to formulate a URL
     used by the RESTful API for retrieving or setting configuration data.
     It is ALSO used to specify a path to the SPIFFS file system
     where configuration data is saved on the MCU board. It should
     ALWAYS start with a forward slash if specified. If left blank,
     that indicates this sensor or transform does not have any
     configuration to save, or that you're not interested in doing
     run-time configuration.
     These two are necessary until a method is created to synthesize them.
  */
  const char* config_path_heading_sk = "/sensors/heading/sk_path";
  const char* config_path_heading = "/sensors/heading/value_settings";
  const char* config_path_attitude_sk = "/sensors/attitude/sk_path";
  const char* config_path_attitude = "/sensors/attitude/value_settings";
  const char* config_path_rateOfTurn_sk = "/sensors/rateOfTurn/sk_path";
  const char* config_path_rateOfTurn = "/sensors/rateOfTurn/value_settings";

  /*
     Above arrangement of config paths yields this web interface structure:

     sensors->heading
                     ->sk-path
                     ->value_settings
             ->attitude
                     ->sk_path
                     ->value_settings
             ->rateOfTurn
                     ->sk_path
                     ->value_settings
   */

  /*
     This example shows heading, attitude (roll, pitch, yaw) and rate of turn.
     If you want other parameters as well, uncomment the appropriate path(s)
     from the following:
  */
  // TODO: Comment below lines before merge into master!
  const char* config_path_accel_x_sk = "/sensors/accel_x/sk-path";
  const char* config_path_accel_x = "/sensors/accel_x/value_settings";
  const char* config_path_accel_y_sk = "/sensors/accel_y/sk-path";
  const char* config_path_accel_y = "/sensors/accel_y/value_settings";
  const char* config_path_accel_z_sk = "/sensors/accel_z/sk_path";
  const char* config_path_accel_z = "/sensors/accel_z/value_settings";
  const char* config_path_turn_rate_sk = "/sensors/turn_rate/sk-path";
  const char* config_path_turn_rate = "/sensors/turn_rate/value_settings";
  const char* config_path_roll_rate_sk = "/sensors/roll_rate/sk_path";
  const char* config_path_roll_rate = "/sensors/roll_rate/value_settings";
  const char* config_path_pitch_rate_sk = "/sensors/pitch_rate/sk_path";
  const char* config_path_pitch_rate = "/sensors/pitch_rate/value_settings";

  // Magnetic Heading data source, using 9 Degrees-of-freedom combination sensor
  Orientation9DOF NXP9DOF =
      Orientation9DOF(PIN_I2C_SDA, PIN_I2C_SCL, config_path_heading);

  /*
     Uncomment the following line during calibration - this will stream raw
     readings to serial port, where they can be intercepted by the Motion Sensor
     Calibration Tool. See Adafruit tutorial
     https://learn.adafruit.com/adafruit-sensorlab-magnetometer-calibration/magnetic-calibration-with-motioncal
     and Paul Stoffregen's app https://github.com/PaulStoffregen/MotionCal
     After calibration values have been written to EEPROM/Flash, re-comment
     following line to restore normal program flow.
  */

  //  NXP9DOF.stream_raw_values();

  /*
     Start periodic readings from orientation sensor. Note that the physical
     sensor is read at whatever rate is specified for the heading
     parameter; all other parameters are retrieved from variables that
     are updated only at the time of the sensor read. So, ensure that
     the sampling rate specified for the heading parameter is the same
     or faster than any of the other parameters (otherwise those other
     parameters will not update at the rate you expect).
  */

  // Heading
  auto* sensor_heading =
      new Read9DOF(&NXP9DOF, compass_hdg, ORIENTATION_SAMPLING_INTERVAL_MS,
                   config_path_heading);

  sensor_heading->connect_to(
      new SKOutputNumber(sk_path_heading, config_path_heading_sk));

  // Attitude
  auto* sensor_attitude = new Read9DOF(
      &NXP9DOF, attitude, ORIENTATION_SAMPLING_INTERVAL_MS * 5, config_path_attitude);

  sensor_attitude->connect_to(
      new SKOutputAttitude(sk_path_attitude, config_path_attitude_sk));

  // Rate of turn
  auto* sensor_rate_of_turn =
      new Read9DOF(&NXP9DOF, rate_of_turn, ORIENTATION_SAMPLING_INTERVAL_MS * 5,
                   config_path_turn_rate);

  sensor_rate_of_turn->connect_to(
      new SKOutputNumber(sk_path_rate_Of_turn, config_path_turn_rate_sk));

  /*
     This example shows heading, attitude (roll, pitch, yaw) and rate of turn.
     If you want other parameters as well, uncomment the appropriate path(s)
     from the following:
  */

  // TODO: Comment below lines before merge into master!

  // Roll rate
  auto* sensor_roll_rate =
      new Read9DOF(&NXP9DOF, rate_of_roll, ORIENTATION_SAMPLING_INTERVAL_MS * 5,
                   config_path_roll_rate);

  sensor_roll_rate->connect_to(
      new SKOutputNumber(sk_path_roll_rate, config_path_roll_rate_sk));

  // Pitch rate
  auto* sensor_pitch_rate = new Read9DOF(&NXP9DOF, rate_of_pitch,
                                         ORIENTATION_SAMPLING_INTERVAL_MS * 5,
                                         config_path_pitch_rate);

  sensor_pitch_rate->connect_to(
      new SKOutputNumber(sk_path_pitch_rate, config_path_pitch_rate_sk));

  // Acceleration X
  auto* sensor_accel_x =
      new Read9DOF(&NXP9DOF, acceleration_x,
                   ORIENTATION_SAMPLING_INTERVAL_MS * 5, config_path_accel_x);

  sensor_accel_x->connect_to(
      new SKOutputNumber(sk_path_accel_x, config_path_accel_x_sk));

  // Acceleration Y
  auto* sensor_accel_y =
      new Read9DOF(&NXP9DOF, acceleration_y,
                   ORIENTATION_SAMPLING_INTERVAL_MS * 5, config_path_accel_y);

  sensor_accel_y->connect_to(
      new SKOutputNumber(sk_path_accel_y, config_path_accel_y_sk));

  // Acceleration Z
  auto* sensor_accel_z =
      new Read9DOF(&NXP9DOF, acceleration_z,
                   ORIENTATION_SAMPLING_INTERVAL_MS * 5, config_path_accel_z);

  sensor_accel_z->connect_to(
      new SKOutputNumber(sk_path_accel_z, config_path_accel_z_sk));

  /*  Relationship of the Axes and the terminology:
  X,Y,Z are orthogonal, in a Right-handed coordinate system:
  think of the X axis pointing West, Y pointing South, and Z pointing up.
  On the Adafruit FXOS8700/FXAS21002 sensor PCB, the axes are printed
  on the top-side silkscreen.
  Acceleration is measured in the direction of the corresponding axis.

  If the sensor is mounted with the X-axis pointing to the bow of the boat
  and the Y-axis pointing to Port, then Z points up and the following applies:
  Heading is rotation about the Z-axis. It increases with rotation to starboard.
  Pitch is rotation about the Y-axis. Positive is when the bow points up.
  Roll is rotation about the X-axis. Positive is rolling to starboard.
  Turn-rate is rotation about the Z-axis. Positive is increasing bearing.
  Roll-rate is rotation about the X-axis. Positive is rolling to starboard.
  Pitch-rate is rotation about the Y-axis. Positive is ***bow falling.

  If the sensor is mounted differently, or you prefer an alternate nomenclature,
  the get___() methods in sensor_NXP_FXOS8700_FXAS21002.cpp can be adjusted.
*/

  // Start the SensESP application running
  sensesp_app->enable();
});