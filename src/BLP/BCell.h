#ifndef _BCell_H_
#define _BCell_H_

#include "sensors/sensor.h"
#include "sensors/ads1x15.h"

class BatteryCell 
{
public:
  BatteryCell(ADS1x15Channel_t channel);               


  float voltage();
  float temperature();
  int health();


};


class BatteryCellConfiguration {
public:
/*! @brief constructor for a battery cell
    @param voltage designed voltage e.g. 3.2V
    @param amph designed amph*hour e.g. 100ah
    @param underVoltage lowest voltage before damage occurs to the cell
    @param overVoltage highest voltage before damage occurs to the cell
    @param warnUnderVoltage low voltage level when a warning signal is send
    @param warnOverVoltage high voltage level when a warning signal is send
    @param maxTemp highest temperature before damage to the cell occurs
    @param warnOverVoltage temperature level where a warning signal is send
  */
  BatteryCellConfiguration( float voltage,
                            float amph, 
                            float underVoltage,
                            float overVoltage,
                            float warnUnderVoltage,
                            float warnOverVoltage,
                            float MaxTemp,
                            float warnMaxTemp
  );

  float voltage();
  float overVoltage();
  float underVoltage();
  float warnOvervoltage();
  float warnUndervoltage();
  float maxTemp();
  float warnTemp();
};

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


#endif