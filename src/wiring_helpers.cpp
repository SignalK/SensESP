#include "wiring_helpers.h"

#include "sensesp_app.h"
#include "devices/analog_input.h"
#include "devices/digital_input.h"
#include "devices/gps.h"
#include "transforms/difference.h"
#include "devices/digital_input.h"
#include "transforms/frequency.h"
#include "signalk/signalk_output.h"
#include "signalk/signalk_position.h"
#include "signalk/signalk_time.h"
#include "transforms/integrator.h"
#include "transforms/linear.h"
#include "transforms/moving_average.h"
#include "transforms/transform.h"


void setup_analog_input(
    String sk_path, float k, float c,
    String config_path) {
  (new AnalogInput())
    -> connectTo(new Linear(k, c, config_path + "/calibrate"))
    -> connectTo(new SignalKNumber(sk_path, config_path + "/sk"));
}


void setup_fuel_flow_meter(
  int inflow_pin,
  int return_flow_pin
) {
  //////////
  // connect a fuel flow meter with return line

  auto* dic1 = new DigitalInputCounter(inflow_pin, INPUT_PULLUP, CHANGE, 1000);
  auto* dic2 = new DigitalInputCounter(return_flow_pin, INPUT_PULLUP, CHANGE, 1000);

  Frequency* freq1;
  Frequency* freq2;

  dic1->connectTo(freq1 = new Frequency())
      -> connectTo(new SignalKNumber("sensors.inflow.frequency"));
    
  dic2->connectTo(freq2 = new Frequency())
      -> connectTo(new SignalKNumber("sensors.outflow.frequency"));


  // Here, each pulse of a flow sensor represents 0.46ml of flow
  // for both inflow and outflow
  auto* diff = new Difference(0.46/1e6, 0.46/1e6,
                              "/sensors/fuel/rate/calibrate");

  diff->connectFrom(freq1, freq2)
      -> connectTo(new SignalKNumber("propulsion.0.fuel.rate", "/sensors/fuel/rate/sk"))
      -> connectTo(new MovingAverage(10, 1., "/sensors/fuel/average/calibrate")) // this is the same as above, but averaged over 10 s
      -> connectTo(new SignalKNumber("propulsion.0.fuel.averageRate", "/sensors/fuel/average/sk"));


  // Integrate the net flow over time. The output is dependent
  // on the the input counter update rate!
  diff->connectTo(new Integrator(1., 0.))
      -> connectTo(new SignalKNumber("propulsion.left.fuel.used", "/sensors/fuel/used/sk"));


  // Integrate the total outflow over time. The output is dependent
  // on the the input counter update rate!
  freq1-> connectTo(new Integrator(0.46/1e6, 0., "/sensors/fuel/in_used/calibrate"))
       -> connectTo(new SignalKNumber("propulsion.left.fuel.usedGross", "/sensors/fuel/in_used/sk"));


  // Integrate the net fuel flow over time. The output is dependent
  // on the the input counter update rate!
  freq2->connectTo(new Integrator(0.46/1e6, 0., "/sensors/fuel/out_used/calibrate"))
       -> connectTo(new SignalKNumber("propulsion.left.fuel.usedReturn", "/sensors/fuel/out_used/sk"));
}


void setup_gps(int serial_input_pin) {
  GPSInput* gps = new GPSInput(serial_input_pin);
  gps->nmea_data.position.connectTo(
    new SignalKPosition("navigation.position", ""));
  gps->nmea_data.gnss_quality.connectTo(
    new SignalKString("navigation.methodQuality", ""));
  gps->nmea_data.num_satellites.connectTo(
    new SignalKInt("navigation.satellites", ""));
  gps->nmea_data.horizontal_dilution.connectTo(
    new SignalKNumber("navigation.horizontalDilution", ""));
  gps->nmea_data.geoidal_separation.connectTo(
    new SignalKNumber("navigation.geoidalSeparation", ""));
  gps->nmea_data.dgps_age.connectTo(
    new SignalKNumber("navigation.differentialAge", ""));
  gps->nmea_data.dgps_id.connectTo(
    new SignalKNumber("navigation.differentialReference", ""));
  gps->nmea_data.datetime.connectTo(
    new SignalKTime("navigation.datetime", ""));
  gps->nmea_data.speed.connectTo(
    new SignalKNumber("navigation.speedOverGround", ""));
  gps->nmea_data.true_course.connectTo(
    new SignalKNumber("navigation.courseOverGroundTrue", ""));
  gps->nmea_data.variation.connectTo(
    new SignalKNumber("navigation.magneticVariation", ""));
  gps->nmea_data.rtk_age.connectTo(
    new SignalKNumber("navigation.rtkAge", ""));
  gps->nmea_data.rtk_ratio.connectTo(
    new SignalKNumber("navigation.rtkRatio", ""));
  gps->nmea_data.baseline_length.connectTo(
    new SignalKNumber("navigation.rtkBaselineLength", ""));
  gps->nmea_data.baseline_course.connectTo(
    new SignalKNumber("navigation.rtkBaselineCourse", ""));
}

void setup_onewire_temperature(
    SensESPApp* seapp,
    DallasTemperatureSensors* dts,
    String sk_path,
    String config_path,
    String schema
) {
  (new OneWireTemperature(dts))->connectTo(
    new SignalKNumber(sk_path, config_path));
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
      -> connectTo<float>(new Frequency(1./97., "/sensors/engine_rpm/calibrate"))
      -> connectTo(new SignalKNumber("propulsion.left.revolutions", "/sensors/engine_rpm/sk"));
  
}
