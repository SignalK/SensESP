#ifndef _BLP_H_
#define _BLP_H_

#include "sensors/sensor.h"
#include "BLP/BCell.h"


/*!
 *  @brief  Class that stores state and functions for battery
 */

/**
 * @brief Represents a LifePo4 battery.
 * 
 * old: The constructor creates a pointer to the sensor and
 * old: starts up the sensor. The pointer is passed to DHTValue, which retrieves
 * old: the specified value.
 * 
 * @param batteryCells specifies a set of cells that together form the battery.
 * 
 * @param batteryCellConfiguration specifies the configuration of all cells.
 * 
 * @param pinTemp the pin where the temperature sensor is connected.
 * 
 * @param pinMinus the pin where the minus/ground is connected on the battery.
 * 
 * @param pinSwitch the pin where the relay  is to switch the battery on or off.
 **/

class BLP
{
public:
  BLP(uint8_t ADSVoltAddress);
  ADS1115* ads1115;
  
  void begin();

  float voltage();
  float overVoltage();
  float warnOverVoltage();
  float underVoltage();
  float warnUnderVoltage();
  float amp();
  float temperature(); // battery Temperature
  float warnTemperature();
  float maxTemperature();
  float chargingVoltageAllowed(); // the allowed maximum voltage for charging
  int SoH(); //State of Health
  int status();
  boolean setSwitch(boolean on); // returns true if turned on
  //float SoC(); // State of Charge
  //float DoD();  //Depth of Decharge
  //int SoP(powerUsage); //the amount of power available for a defined time interval given the current power usage, temperature and other condition

  BatteryCell* BC[];
 // BatteryCellConfiguration BCConfig;

private:
  float _volt;
  float _overVoltage;
  float _underVoltage;
  float _warnOvervoltage;
  float _warnUndervoltage;
  float _amp;
  float _temperature;
  float _warnTemperature;
  float _maxTemperature;
  int _pinTemp;
  int _pinRelay;
  int _pinMinus;
};

#endif