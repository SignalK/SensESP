#include "BCell.h"

#include "sensesp.h"
#include <RemoteDebug.h>

ADS1x15Channel_t channel;
int pinPlus;
int pinTemp;

float r1; //resistor 1 of voltage divider
float r2; //resistor 2 of voltage divider

BatteryCell::BatteryCell(ADS1x15Channel_t _channel)
{
  channel=_channel;

};               

BatteryCellConfiguration::BatteryCellConfiguration(     float voltage,
                                                        float amph, 
                                                        float underVoltage,
                                                        float overVoltage,
                                                        float warnUnderVoltage,
                                                        float warnOverVoltage,
                                                        float MaxTemp,
                                                        float warnMaxTemp)
  
  {

  };

float BatteryCell::voltage()
{
    int read=analogRead(pinPlus);
    float v=read*5/4096*(r1+r2)/r2;
  return v;
}

