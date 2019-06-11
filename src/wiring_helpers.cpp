#include "wiring_helpers.h"

#include "sensesp_app.h"
#include "devices/analog_input.h"
#include "devices/digital_input.h"
#include "devices/gps.h"
#include "transforms/difference.h"
#include "devices/digital_input.h"
#include "transforms/frequency.h"
#include "transforms/gnss_position.h"
#include "transforms/integrator.h"
#include "transforms/linear.h"
#include "transforms/moving_average.h"
#include "transforms/timestring.h"
#include "transforms/transform.h"
#include "transforms/passthrough.h"


void setup_analog_input(
    String sk_path, float k, float c,
    String config_path) {
  (new AnalogInput())
    -> connectTo(new Linear(sk_path, k, c, config_path));
}

void setup_fuel_flow_meter(
  int inflow_pin,
  int return_flow_pin
) {
  //////////
  // connect a fuel flow meter with return line

  auto* dic1 = new DigitalInputCounter(inflow_pin, INPUT_PULLUP, CHANGE, 1000);
  auto* dic2 = new DigitalInputCounter(return_flow_pin, INPUT_PULLUP, CHANGE, 1000);

  auto* freq1 = new Frequency("sensors.inflow.frequency");
  auto* freq2 = new Frequency("sensors.outflow.frequency");

  dic1->connectTo(freq1);
  dic2->connectTo(freq2);

  // Here, each pulse of a flow sensor represents 0.46ml of flow
  // for both inflow and outflow
  auto* diff = new Difference("propulsion.0.fuel.rate",
                                    0.46/1e6, 0.46/1e6,
                                    "/sensors/fuel_rate");

  diff->connectFrom(freq1, freq2);

  // this is the same as above, but averaged over 10 s
  diff->connectTo(
    new MovingAverage("propulsion.0.fuel.averageRate", 10, 1., "/sensors/fuel_average_rate"));

  // Integrate the net flow over time. The output is dependent
  // on the the input counter update rate!
  diff->connectTo(
    new Integrator("propulsion.left.fuel.used", 1., 0., "/sensors/fuel_used"));

  // Integrate the total outflow over time. The output is dependent
  // on the the input counter update rate!
  freq1->connectTo(
    new Integrator("propulsion.left.fuel.usedGross", 0.46/1e6, 0., "/sensors/fuel_in_used"));

  // Integrate the net fuel flow over time. The output is dependent
  // on the the input counter update rate!
  freq2->connectTo(
    new Integrator("propulsion.left.fuel.usedReturn", 0.46/1e6, 0., "/sensors/fuel_out_used"));
}

void setup_gps(int serial_input_pin) {
  GPSInput* gps = new GPSInput(serial_input_pin);
  gps->nmea_data.position.connectTo(
    new GNSSPosition("navigation.position", ""));
  gps->nmea_data.gnss_quality.connectTo(
    new Passthrough<String>("navigation.methodQuality", ""));
  gps->nmea_data.num_satellites.connectTo(
    new Passthrough<int>("navigation.satellites", ""));
  gps->nmea_data.horizontal_dilution.connectTo(
    new Passthrough<float>("navigation.horizontalDilution", ""));
  gps->nmea_data.geoidal_separation.connectTo(
    new Passthrough<float>("navigation.geoidalSeparation", ""));
  gps->nmea_data.dgps_age.connectTo(
    new Passthrough<float>("navigation.differentialAge", ""));
  gps->nmea_data.dgps_id.connectTo(
    new Passthrough<float>("navigation.differentialReference", ""));
  gps->nmea_data.datetime.connectTo(
    new TimeString("navigation.datetime", ""));
  gps->nmea_data.speed.connectTo(
    new Passthrough<float>("navigation.speedOverGround", ""));
  gps->nmea_data.true_course.connectTo(
    new Passthrough<float>("navigation.courseOverGroundTrue", ""));
  gps->nmea_data.variation.connectTo(
    new Passthrough<float>("navigation.magneticVariation", ""));
  gps->nmea_data.rtk_age.connectTo(
    new Passthrough<float>("navigation.rtkAge", ""));
  gps->nmea_data.rtk_ratio.connectTo(
    new Passthrough<float>("navigation.rtkRatio", ""));
  gps->nmea_data.baseline_length.connectTo(
    new Passthrough<float>("navigation.rtkBaselineLength", ""));
  gps->nmea_data.baseline_course.connectTo(
    new Passthrough<float>("navigation.rtkBaselineCourse", ""));
}

void setup_onewire_temperature(
    SensESPApp* seapp,
    DallasTemperatureSensors* dts,
    String sk_path,
    String config_path,
    String schema
) {
  (new OneWireTemperature(dts))->connectTo(
    new Passthrough<float>(sk_path, config_path));
}

void setup_rpm_meter(SensESPApp* seapp, int input_pin) {
  //////////
  // connect a RPM meter. A DigitalInputCounter counts pulses
  // and reports the readings every read_delay ms
  // (500 in the example). A Frequency
  // transform takes a number of pulses and converts that into
  // a frequency. The sample multiplier converts the 97 tooth
  // tach output into Hz, SK native units.

  (new DigitalInputCounter(input_pin, INPUT_PULLUP, RISING, 500))
    -> connectTo(new Frequency("propulsion.left.revolutions", 1./97., "/sensors/engine_rpm"));
}
