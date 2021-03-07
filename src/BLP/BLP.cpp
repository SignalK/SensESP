#include "BLP.h"
#include "BCell.h"
#include "sensesp.h"
#include <RemoteDebug.h>




// BLP represents a LifePo4 battery.
BLP::BLP (uint8_t ADSVoltAddress)
{
  adsGain_t gain = GAIN_TWOTHIRDS;
  ADS1115* ads1115 = new ADS1115 (ADSVoltAddress, gain);
    
    //first initialize and order the channels according to increasing voltage
    //NOT YET IMPLEMENTED

    BC[1]= new BatteryCell( channel_0);
    BC[2]= new BatteryCell( channel_1);
    BC[3]= new BatteryCell( channel_2);
    BC[4]= new BatteryCell( channel_3);
    _pinMinus=1;
    _pinTemp=2;
    _pinRelay=3;
    
  //  BCConfig =  BatteryCellConfiguration(1,2,3,4,5,6,7,8 );  



    // _overVoltage=4*batteryCellConfiguration.overVoltage(); 
    // _warnOvervoltage=numberOfCells*batteryCellConfiguration.warnOvervoltage();

    // _underVoltage=numberOfCells*batteryCellConfiguration.underVoltage();
    // _warnUndervoltage=numberOfCells*batteryCellConfiguration.warnUndervoltage();

    // _warnTemperature=batteryCellConfiguration.warnTemp();
    // _maxTemperature=batteryCellConfiguration.maxTemp();
    
    pinMode(_pinTemp,INPUT); // check temperature sensor type uses analog pin
    pinMode(_pinMinus, INPUT);
    pinMode(_pinRelay, OUTPUT);
};
float BLP::voltage(){
    
    float v=0.0;// BC[1].voltage()+BC[2].voltage()+BC[3].voltage().BC[4].voltage();
    return v;
};

float BLP::overVoltage()
{
    return _overVoltage;
};

  float BLP::warnOverVoltage(){

  };
  float BLP::underVoltage(){

  };
  float BLP::warnUnderVoltage(){

  };
  float BLP::amp(){

  };
  float BLP::temperature(){

  }; // battery Temperature
  float BLP::warnTemperature(){

  };
  float BLP::maxTemperature(){

  };
  float BLP::chargingVoltageAllowed(){
    return 0.0;
  }; // the allowed maximum voltage for charging
  int BLP::SoH(){

  }; //State of Health
  int BLP::status(){
      
  };
  boolean BLP::setSwitch(boolean on){
    return true;
  }; // returns true if turned on

