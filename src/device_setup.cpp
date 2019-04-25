#include "app.h"
#include "devices/analog_input.h"
#include "devices/digital_input.h"
#include "devices/onewire_temperature.h"
#include "devices/gps.h"
#include "transforms/transform.h"
#include "transforms/difference.h"
#include "transforms/frequency.h"
#include "transforms/integrator.h"
#include "transforms/linear.h"
#include "transforms/timestring.h"
#include "transforms/moving_average.h"
#include "transforms/gnss_position.h"

void SensESPApp::setup_custom_devices() {

  //////////
  // connect an analog input

  connect_1to1<AnalogInput, Passthrough<float>>(
    new AnalogInput(),
    new Passthrough<float>("sensors.indoor.illumination"));

  // expose a slowly changing (button-type) digital input
#if 0
  DigitalInputValue* digin = new DigitalInputValue(D1, INPUT_PULLUP, RISING);
  connect_1to1<DigitalInputValue, Passthrough<bool>>(
    digin,
    new Passthrough<bool>("sensors.sensesp.button")
  );
#endif
  //////////
  // connect a fuel flow meter with return line

  Frequency* f1 = new Frequency("sensors.inflow.frequency");
  Frequency* f2 = new Frequency("sensors.outflow.frequency");
  connect_1to1<DigitalInputCounter, Frequency>(
    new DigitalInputCounter(D1, INPUT_PULLUP, CHANGE, 1000),
    f1
  );

  connect_1to1<DigitalInputCounter, Frequency>(
    new DigitalInputCounter(D2, INPUT_PULLUP, CHANGE, 1000),
    f2
  );

  // Here, each pulse of a flow sensor represents 0.46ml of flow
  // for both inflow and outflow
  Difference* diff = new Difference("propulsion.left.fuel.rate",
                                    0.46/1e6, 0.46/1e6,
                                    "/sensors/fuel_rate");
  connect_2to1<Frequency, Difference>(
    f1, f2,
    diff
  );

  // this is the same as above, but averaged over 10 s
  connect_1to1<Difference, MovingAverage>(
    diff,
    new MovingAverage("propulsion.left.fuel.averageRate", 10, 1., "/sensors/fuel_average_rate")
  );


  // Integrate the net flow over time. The output is dependent
  // on the the input counter update rate!
  connect_1to1<Difference, Integrator>(
    diff,
    new Integrator("propulsion.left.fuel.used", 1., 0., "/sensors/fuel_used")
  );

  // Integrate the total outflow over time. The output is dependent
  // on the the input counter update rate!
  connect_1to1<Frequency, Integrator>(
    f1,
    new Integrator("propulsion.left.fuel.usedGross", 0.46/1e6, 0., "/sensors/fuel_in_used")
  );

  // Integrate the net fuel flow over time. The output is dependent
  // on the the input counter update rate!
  connect_1to1<Frequency, Integrator>(
    f2,
    new Integrator("propulsion.left.fuel.usedReturn", 0.46/1e6, 0., "/sensors/fuel_out_used")
  );

#if 0
  //////////
  // connect a RPM meter. A DigitalInputCounter counts pulses
  // and reports the readings every read_delay ms
  // (500 in the example). A Frequency
  // transform takes a number of pulses and converts that into
  // a frequency. The sample multiplier converts the 97 tooth
  // tach output into Hz, SK native units.

  connect_1to1<DigitalInputCounter, Frequency>(
    new DigitalInputCounter(D5, INPUT_PULLUP, RISING, 500),
    new Frequency("propulsion.left.revolutions", 1./97., "/sensors/engine_rpm")
  );
  #endif

#if 0
  //////////
  // OneWire temperature devices need to have the bus
  // controller device created first
  DallasTemperatureSensors* dts = new DallasTemperatureSensors(D7);

  connect_1to1<OneWireTemperature, Passthrough<float>>(
    new OneWireTemperature(dts, "/devices/ow_temp_1", ""),
    new Passthrough<float>("sensors.temperature.ow1",
                           "/sensors/ow_temp_1"));
#endif

  GPSInput* gps = new GPSInput(D5);

  connect_1to1<ObservableValue<Position>, GNSSPosition>(
    &(gps->nmea_data.position),
    new GNSSPosition("navigation.position", "")
  );

  connect_1to1<ObservableValue<String>, Passthrough<String>>(
    &(gps->nmea_data.gnss_quality),
    new Passthrough<String>("navigation.methodQuality", "")
  );

  connect_1to1<ObservableValue<int>, Passthrough<int>>(
    &(gps->nmea_data.num_satellites),
    new Passthrough<int>("navigation.satellites", "")
  );

  connect_1to1<ObservableValue<float>, Passthrough<float>>(
    &(gps->nmea_data.horizontal_dilution),
    new Passthrough<float>("navigation.horizontalDilution", "")
  );

  connect_1to1<ObservableValue<float>, Passthrough<float>>(
    &(gps->nmea_data.geoidal_separation),
    new Passthrough<float>("navigation.geoidalSeparation", "")
  );

  connect_1to1<ObservableValue<float>, Passthrough<float>>(
    &(gps->nmea_data.dgps_age),
    new Passthrough<float>("navigation.differentialAge", "")
  );

  connect_1to1<ObservableValue<float>, Passthrough<float>>(
    &(gps->nmea_data.dgps_id),
    new Passthrough<float>("navigation.differentialReference", "")
  );

  connect_1to1<ObservableValue<time_t>, TimeString>(
    &(gps->nmea_data.datetime),
    new TimeString("navigation.datetime", "")
  );

  connect_1to1<ObservableValue<float>, Passthrough<float>>(
    &(gps->nmea_data.speed),
    new Passthrough<float>("navigation.speedOverGround", "")
  );

  connect_1to1<ObservableValue<float>, Passthrough<float>>(
    &(gps->nmea_data.true_course),
    new Passthrough<float>("navigation.courseOverGroundTrue", "")
  );

  connect_1to1<ObservableValue<float>, Passthrough<float>>(
    &(gps->nmea_data.variation),
    new Passthrough<float>("navigation.magneticVariation", "")
  );

  connect_1to1<ObservableValue<float>, Passthrough<float>>(
    &(gps->nmea_data.rtk_age),
    new Passthrough<float>("navigation.rtkAge", "")
  );

  connect_1to1<ObservableValue<float>, Passthrough<float>>(
    &(gps->nmea_data.rtk_ratio),
    new Passthrough<float>("navigation.rtkRatio", "")
  );

  connect_1to1<ObservableValue<float>, Passthrough<float>>(
    &(gps->nmea_data.baseline_length),
    new Passthrough<float>("navigation.rtkBaselineLength", "")
  );

  connect_1to1<ObservableValue<float>, Passthrough<float>>(
    &(gps->nmea_data.baseline_course),
    new Passthrough<float>("navigation.rtkBaselineCourse", "")
  );



}
