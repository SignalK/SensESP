#include "wiring_helpers.h"

#include "sensesp_app.h"
#include "sensors/analog_input.h"
#include "sensors/digital_input.h"
#include "sensors/gps.h"
#include "transforms/difference.h"
#include "sensors/digital_input.h"
#include "transforms/angle_correction.h"
#include "transforms/frequency.h"
#include "signalk/signalk_output.h"
#include "signalk/signalk_position.h"
#include "signalk/signalk_time.h"
#include "transforms/integrator.h"
#include "transforms/linear.h"
#include "transforms/moving_average.h"
#include "transforms/transform.h"


// Obsolete.
void setup_analog_input(
    String sk_path, float k, float c,
    String config_path) {
  (new AnalogInput())
    -> connect_to(new Linear(k, c, config_path + "/calibrate"))
    -> connect_to(new SKOutputNumber(sk_path, config_path + "/sk"));
}


void setup_fuel_flow_meter(
  int inflow_pin,
  int return_flow_pin
) {
  //////////
  // connect a fuel flow meter with return line

  auto* dicIn = new DigitalInputCounter(inflow_pin, INPUT_PULLUP, CHANGE, 1000);
  auto* dicOut = new DigitalInputCounter(return_flow_pin, INPUT_PULLUP, CHANGE, 1000);

  Frequency* freqIn;
  Frequency* freqOut;

  dicIn->connect_to(freqIn = new Frequency())
      -> connect_to(new SKOutputNumber("fuelflow.inflow.frequency"));

  dicOut->connect_to(freqOut = new Frequency())
      -> connect_to(new SKOutputNumber("fuelflow.outflow.frequency"));


  // Here, each pulse of a flow sensor represents 0.46ml of flow
  // for both inflow and outflow
  auto* diff = new Difference(0.46/1e6, 0.46/1e6,
                              "/fuelflow/fuel/rate/calibrate");

  diff->connectFrom(freqIn, freqOut)
      -> connect_to(new SKOutputNumber("propulsion.main.fuel.rate", "/fuelflow/fuel/rate/sk"))
      -> connect_to(new MovingAverage(10, 1., "/fuelflow/fuel/average/calibrate")) // this is the same as above, but averaged over 10 s
      -> connect_to(new SKOutputNumber("propulsion.main.fuel.averageRate", "/fuelflow/fuel/average/sk"));


  // Integrate the net flow over time. The output is dependent
  // on the the input counter update rate!
  diff->connect_to(new Integrator(1., 0.))
      -> connect_to(new SKOutputNumber("propulsion.main.fuel.used", "/fuelflow/fuel/used/sk"));


  // Integrate the total outflow over time. The output is dependent
  // on the the input counter update rate!
  freqIn-> connect_to(new Integrator(0.46/1e6, 0., "/fuelflow/fuel/in_used/calibrate"))
       -> connect_to(new SKOutputNumber("propulsion.main.fuel.usedGross", "/fuelflow/fuel/in_used/sk"));


  // Integrate the net fuel flow over time. The output is dependent
  // on the the input counter update rate!
  freqOut->connect_to(new Integrator(0.46/1e6, 0., "/fuelflow/fuel/out_used/calibrate"))
       -> connect_to(new SKOutputNumber("propulsion.main.fuel.usedReturn", "/fuelflow/fuel/out_used/sk"));
}


GPSInput* setup_gps(Stream* rx_stream) {
  GPSInput* gps = new GPSInput(rx_stream);
  gps->nmea_data.position
    .connect_to(new SKOutputPosition("navigation.position", ""));
  gps->nmea_data.gnss_quality
    .connect_to(new SKOutputString("navigation.methodQuality", ""));
  gps->nmea_data.num_satellites
    .connect_to(new SKOutputInt("navigation.satellites", ""));
  gps->nmea_data.horizontal_dilution
    .connect_to(new SKOutputNumber("navigation.horizontalDilution", ""));
  gps->nmea_data.geoidal_separation
    .connect_to(new SKOutputNumber("navigation.geoidalSeparation", ""));
  gps->nmea_data.dgps_age
    .connect_to(new SKOutputNumber("navigation.differentialAge", ""));
  gps->nmea_data.dgps_id
    .connect_to(new SKOutputNumber("navigation.differentialReference", ""));
  gps->nmea_data.datetime
    .connect_to(new SKOutputTime("navigation.datetime", ""));
  gps->nmea_data.speed
    .connect_to(new SKOutputNumber("navigation.speedOverGround", ""));
  gps->nmea_data.true_course
    .connect_to(new SKOutputNumber("navigation.courseOverGroundTrue", ""));
  gps->nmea_data.variation
    .connect_to(new SKOutputNumber("navigation.magneticVariation", ""));
  gps->nmea_data.rtk_age
    .connect_to(new SKOutputNumber("navigation.rtkAge", ""));
  gps->nmea_data.rtk_ratio
    .connect_to(new SKOutputNumber("navigation.rtkRatio", ""));
  gps->nmea_data.baseline_length
    .connect_to(new SKOutputNumber("navigation.rtkBaselineLength", ""));
  gps->nmea_data.baseline_course
    .connect_to(new SKOutputNumber("navigation.rtkBaselineCourse"))
    ->connect_to(new AngleCorrection(0, 0, "/sensors/heading/correction"))
    ->connect_to(new SKOutputNumber("navigation.headingTrue", ""));

  return gps;
}

//Obsolete
void setup_onewire_temperature(
    SensESPApp* seapp,
    DallasTemperatureSensors* dts,
    String sk_path,
    String config_path,
    String schema
) {
  (new OneWireTemperature(dts))->connect_to(
    new SKOutputNumber(sk_path, config_path));
}

//Obsolete
void setup_rpm_meter(SensESPApp* seapp, int input_pin) {
  //////////
  // connect a RPM meter. A DigitalInputCounter counts pulses
  // and reports the readings every read_delay ms
  // (500 in the example). A Frequency
  // transform takes a number of pulses and converts that into
  // a frequency. The sample multiplier converts the 97 tooth
  // tach output into Hz, SK native units.

  (new DigitalInputCounter(input_pin, INPUT_PULLUP, RISING, 500))
      -> connect_to<float>(new Frequency(1./97., "/sensors/engine_rpm/calibrate"))
      -> connect_to(new SKOutputNumber("propulsion.main.revolutions", "/sensors/engine_rpm/sk"));

}
