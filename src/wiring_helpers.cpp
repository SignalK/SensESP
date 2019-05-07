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


void setup_analog_input(
    SensESPApp* seapp,
    String sk_path, float k, float c,
    String id, String schema) {
  seapp->connect_1to1<AnalogInput, Linear>(
    new AnalogInput(),
    new Linear(
      sk_path, k, c,
      id, schema));
}

void setup_digital_input(
    SensESPApp* seapp,
    int digital_pin,
    String sk_path, float k, float c,
    String id, String schema) {
  // expose a slowly changing (button-type) digital input
  seapp->connect_1to1<DigitalInputValue, Linear>(
    new DigitalInputValue(digital_pin, INPUT_PULLUP, RISING),
    new Linear(
      sk_path, k, c,
      id, schema));
}

void setup_fuel_flow_meter(
  SensESPApp* seapp,
  int inflow_pin,
  int return_flow_pin
) {
  //////////
  // connect a fuel flow meter with return line

  Frequency* f1 = new Frequency("sensors.inflow.frequency");
  Frequency* f2 = new Frequency("sensors.outflow.frequency");
  seapp->connect_1to1<DigitalInputCounter, Frequency>(
    new DigitalInputCounter(inflow_pin, INPUT_PULLUP, CHANGE, 1000),
    f1
  );

  seapp->connect_1to1<DigitalInputCounter, Frequency>(
    new DigitalInputCounter(return_flow_pin, INPUT_PULLUP, CHANGE, 1000),
    f2
  );

  // Here, each pulse of a flow sensor represents 0.46ml of flow
  // for both inflow and outflow
  Difference* diff = new Difference("propulsion.left.fuel.rate",
                                    0.46/1e6, 0.46/1e6,
                                    "/sensors/fuel_rate");
  seapp->connect_2to1<Frequency, Difference>(
    f1, f2,
    diff
  );

  // this is the same as above, but averaged over 10 s
  seapp->connect_1to1<Difference, MovingAverage>(
    diff,
    new MovingAverage("propulsion.left.fuel.averageRate", 10, 1., "/sensors/fuel_average_rate")
  );


  // Integrate the net flow over time. The output is dependent
  // on the the input counter update rate!
  seapp->connect_1to1<Difference, Integrator>(
    diff,
    new Integrator("propulsion.left.fuel.used", 1., 0., "/sensors/fuel_used")
  );

  // Integrate the total outflow over time. The output is dependent
  // on the the input counter update rate!
  seapp->connect_1to1<Frequency, Integrator>(
    f1,
    new Integrator("propulsion.left.fuel.usedGross", 0.46/1e6, 0., "/sensors/fuel_in_used")
  );

  // Integrate the net fuel flow over time. The output is dependent
  // on the the input counter update rate!
  seapp->connect_1to1<Frequency, Integrator>(
    f2,
    new Integrator("propulsion.left.fuel.usedReturn", 0.46/1e6, 0., "/sensors/fuel_out_used")
  );
}

void setup_gps(SensESPApp* seapp, int serial_input_pin) {
  GPSInput* gps = new GPSInput(serial_input_pin);

  seapp->connect_1to1<ObservableValue<Position>, GNSSPosition>(
    &(gps->nmea_data.position),
    new GNSSPosition("navigation.position", "")
  );

  seapp->connect_1to1<ObservableValue<String>, Passthrough<String>>(
    &(gps->nmea_data.gnss_quality),
    new Passthrough<String>("navigation.methodQuality", "")
  );

  seapp->connect_1to1<ObservableValue<int>, Passthrough<int>>(
    &(gps->nmea_data.num_satellites),
    new Passthrough<int>("navigation.satellites", "")
  );

  seapp->connect_1to1<ObservableValue<float>, Passthrough<float>>(
    &(gps->nmea_data.horizontal_dilution),
    new Passthrough<float>("navigation.horizontalDilution", "")
  );

  seapp->connect_1to1<ObservableValue<float>, Passthrough<float>>(
    &(gps->nmea_data.geoidal_separation),
    new Passthrough<float>("navigation.geoidalSeparation", "")
  );

  seapp->connect_1to1<ObservableValue<float>, Passthrough<float>>(
    &(gps->nmea_data.dgps_age),
    new Passthrough<float>("navigation.differentialAge", "")
  );

  seapp->connect_1to1<ObservableValue<float>, Passthrough<float>>(
    &(gps->nmea_data.dgps_id),
    new Passthrough<float>("navigation.differentialReference", "")
  );

  seapp->connect_1to1<ObservableValue<time_t>, TimeString>(
    &(gps->nmea_data.datetime),
    new TimeString("navigation.datetime", "")
  );

  seapp->connect_1to1<ObservableValue<float>, Passthrough<float>>(
    &(gps->nmea_data.speed),
    new Passthrough<float>("navigation.speedOverGround", "")
  );

  seapp->connect_1to1<ObservableValue<float>, Passthrough<float>>(
    &(gps->nmea_data.true_course),
    new Passthrough<float>("navigation.courseOverGroundTrue", "")
  );

  seapp->connect_1to1<ObservableValue<float>, Passthrough<float>>(
    &(gps->nmea_data.variation),
    new Passthrough<float>("navigation.magneticVariation", "")
  );

  seapp->connect_1to1<ObservableValue<float>, Passthrough<float>>(
    &(gps->nmea_data.rtk_age),
    new Passthrough<float>("navigation.rtkAge", "")
  );

  seapp->connect_1to1<ObservableValue<float>, Passthrough<float>>(
    &(gps->nmea_data.rtk_ratio),
    new Passthrough<float>("navigation.rtkRatio", "")
  );

  seapp->connect_1to1<ObservableValue<float>, Passthrough<float>>(
    &(gps->nmea_data.baseline_length),
    new Passthrough<float>("navigation.rtkBaselineLength", "")
  );

  seapp->connect_1to1<ObservableValue<float>, Passthrough<float>>(
    &(gps->nmea_data.baseline_course),
    new Passthrough<float>("navigation.rtkBaselineCourse", "")
  );
}

void setup_onewire_temperature(
    SensESPApp* seapp,
    DallasTemperatureSensors* dts,
    String sk_path,
    String id,
    String schema
) {
  seapp->connect_1to1<OneWireTemperature, Passthrough<float>>(
    new OneWireTemperature(dts),
    new Passthrough<float>(sk_path, id, schema));
}

void setup_rpm_meter(SensESPApp* seapp, int input_pin) {
  //////////
  // connect a RPM meter. A DigitalInputCounter counts pulses
  // and reports the readings every read_delay ms
  // (500 in the example). A Frequency
  // transform takes a number of pulses and converts that into
  // a frequency. The sample multiplier converts the 97 tooth
  // tach output into Hz, SK native units.

  seapp->connect_1to1<DigitalInputCounter, Frequency>(
    new DigitalInputCounter(input_pin, INPUT_PULLUP, RISING, 500),
    new Frequency("propulsion.left.revolutions", 1./97., "/sensors/engine_rpm")
  );
}
