#ifndef tee_H
#define tee_H

#include "transforms/transform.h"

/**
 * from https://isocpp.org/wiki/faq/pointers-to-members#fnptr-vs-memfnptr-types
 * Allows call to an external function (e.g. one defined in main.cpp), which then has job of manipulating 
 * a copy of the data passing through the tee. Alternative would be to expose member fcn 
 * to the Tee class, and pass in reference to instance in the connectTo() method of main.cpp.
 * This latter approach seemed difficult.
 */
 typedef  void (*WrapperFcn)(float x);  

/**
 * Tee is a numeric passthrough transform that always passes the value unchanged. 
 * It also optionally applies a linear transform of the form y = mx + b, and then passes the y value
 *  to an external function: tee_to().  This external function can be used, for example, to output the
 *  y value in the form of a frequency-modulated signal on a GPIO pin.
 * 
 * The Tee class is so called because it inserts a tee, or tap, into the string of value Producers/Consumers.
 * How to use: 
 *   #include tee.h in main.cpp.  
 *   Define a class that manipulates the data however you want. For example, a WaveformGen class that outputs
 *     a square wave on a GPIO of a frequency proportional to the data.
 *   In main.cpp, declare a wrapper function that invokes your data output class:
 *     WaveformGen* pTachOutputObj; // Wrapper function uses a global to remember the object:
 *     void TeeOutput_wrapper( float new_freq )
 *     {    //Changes the output frequency to new_freq using WaveformGen member set_freq()
 *          pTachOutputObj->set_freq( (int)(new_freq) );
 *          debugI( "f= %d", (int)(new_freq) );
 *     }
 *  Finally, in main.cpp where you setup your data Producer/Consumer chain:
 *      pSensor_tach->connectTo(new Frequency( multiplier_tach, config_path_rpm_calibrate)) // connect the output of pSensor to the input of Frequency()
 *          ->connectTo(new Tee( true, 1.0, 1.0, TeeOutput_wrapper, config_path_rpm_tee ))  // connect the output via a tee to waveform generator
 *          ->connectTo(new SKOutputNumber( sk_path_rpm, config_path_rpm_skpath ));         // connect the output to SignalK Output as a number
 * 
 */
class Tee : public NumericTransform {

    public:
        Tee(uint16_t isTeeActive, float coefficient_tap, float offset_Tap, void (*pFunc_tap)(float x), String config_path="");
        virtual void set_input(float input, uint8_t inputChannel = 0) override;
        virtual JsonObject& get_configuration(JsonBuffer& buf) override;
        virtual bool set_configuration(const JsonObject& config) override;
        virtual String get_config_schema() override;

    private:
        uint16_t isTeeActive;
        float coefficient_tap;
        float offset_tap;
        WrapperFcn tee_to;
};


#endif
