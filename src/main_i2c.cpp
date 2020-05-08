#include <Arduino.h>
// modified

#include <sensesp_app.h>
#include <sensors/i2c_input.h>
#include <transforms/linear.h>
#include <signalk/signalk_output.h>

// SensESP builds upon the ReactESP framework. Every ReactESP application
// defines an "app" object vs defining a "main()" method.
ReactESP app([] () {
int i;
int regsize;
char buf[128];
	// Some initialization boilerplate when in debug mode...
	#ifndef SERIAL_DEBUG_DISABLED
	Serial.begin(115200);

	// A small arbitrary delay is required to let the
	// serial port catch up
	delay(100);
	Debug.setSerialEnabled(true);
	#endif



	// Create the global SensESPApp() object.
	sensesp_app = new SensESPApp(uptimeOnly);


	// The "SignalK path" identifies this sensor to the SignalK network. Leaving
	// this blank would indicate this particular sensor (or transform) does not
	// broadcast SignalK data
	const char* sk_path = "sensors.KDS.i2C";


	// The "Configuration path" is combined with "/config" to formulate a URL
	// used by the RESTful API for retrieving or setting configuration data.
	// It is ALSO used to specify a path to the SPIFFS file system
	// where configuration data is saved on the MCU board.  It should
	// ALWAYS start with a forward slash if specified.  If left blank,
	// that indicates this sensor or transform does not have any
	// configuration to save.
	// Note that if you want to be able to change the sk_path at runtime,
	// you will need to specify a config_path.
//  const char* config_path = "/sensors/KDS/CMPS12/bearing16";
	const char* config_path = "/sensors/KDS/CMPS12/";
//const char* config_path2 = "/sensors/KDS_digital";


	// Create a "sensor" that is the source of our data
	
	
	
	
	//auto* pI2CInput = new I2CInput(0x60,2, config_path);
//  auto* pSensor = new DigitalInputValue(D5, INPUT_PULLUP, RISING);
	//auto* pI2CInput2 = new I2CInput(0x60,26, "/sensors/KDS/CMPS12/BOSCH-bearing16");
	
	// Create a "transform" that can modify the data and/or broadcast it
	// over the SignalK network.
	const float multiplier = 1.0;
	const float offset = 0.0;
//  auto* pTransform = new Linear(multiplier, offset, config_path);


	// Wire up the output of the analog input to the transform,
	// and then output the results on the SignalK network...
//  Serial.printf("Connect to new I2C Input\n");
/*
	for(i=0; i < 31; i++)
	{
		char buf[128];
		switch(i)
		{
// 16 Bit Register      
			case 2:
			{
				sprintf(buf, "%sBearing16",config_path);
				regsize = 16;        
		auto* pI2CInput2 = new I2CInput(0x60,i, buf, regsize);
		pI2CInput2 -> connectTo(new SKOutputNumber(buf));
//        auto* pI2CInput = new I2CInput(0x60,i, buf, regsize);
//        pI2CInput -> connectTo(new SKOutputNumber(buf));
			}
				break;
			case 6:
			{
				sprintf(buf, "%sMagX",config_path);
				regsize = 16;        
			}
				break;
			case 8:
			{
				sprintf(buf, "%sMagY",config_path);
				regsize = 16;        
			}
				break;
			case 10:
			{
				sprintf(buf, "%sMagZ",config_path);
				regsize = 16;        
			}
				break;
			case 12:
			{
				sprintf(buf, "%sAX",config_path);
				regsize = 16;        
			}
				break;
			case 14:
			{
				sprintf(buf, "%sAY",config_path);
				regsize = 16;        
			}
				break;
			case 16:
			{
				sprintf(buf, "%sAZ",config_path);
				regsize = 16;        
			}
				break;
			case 18:
			{
				sprintf(buf, "%sGX",config_path);
				regsize = 16;        
			}
				break;
			case 20:
			{
				sprintf(buf, "%sGY",config_path);
				regsize = 16;        
			}
				break;
			case 22:
			{
				sprintf(buf, "%sGZ",config_path);
				regsize = 16;        
			}
				break;
			case 24:
			{
				sprintf(buf, "%sTemp",config_path);
				regsize = 16;        
		auto* pI2CInput2 = new I2CInput(0x60,i, buf, regsize);
		pI2CInput2 -> connectTo(new SKOutputNumber(buf));
			}
				break;
			case 26:
			{
				sprintf(buf, "%sBearingBosch",config_path);
				regsize = 16;        
		auto* pI2CInput2 = new I2CInput(0x60,i, buf, regsize);
		pI2CInput2 -> connectTo(new SKOutputNumber(buf));
			}
				break;
			case 28:
			{
				sprintf(buf, "%sPitch16",config_path,i);
				regsize = 16;        
				i++;
			}
				break;
			// 8 Bit Register
			case 0:
			{
				sprintf(buf, "%sSoftwareVers",config_path);
				regsize = 8;        
			}
				break;
			case 1:
			{
				sprintf(buf, "%sBearing8",config_path);
				regsize = 8;        
			}
				break;
			case 4:
			{
				sprintf(buf, "%sPitch8",config_path);
				regsize = 8;        
			}
				break;
			case 5:
			{
				sprintf(buf, "%sRoll",config_path);
				regsize = 8;        
			}
				break;
			case 30:
			{
				sprintf(buf, "%sCalibration",config_path);
				regsize = 8;        
			}
				break;;
		}

		auto* pI2CInput2 = new I2CInput(0x60,i, buf, regsize);
		pI2CInput2 -> connectTo(new SKOutputNumber(buf));

	}
*/
	//pI2CInput -> connectTo(new SKOutputNumber(config_path));
//  pDigitalInput -> connectTo(new SKOutputNumber(sk_path));
//  Serial.printf("Connection done to new I2C Input\n");
		I2CInput* pI2CInput;
		sprintf(buf, "%sBearing16",config_path);
		pI2CInput = new I2CInput(0x60,2, buf, 16);
		auto* pTransform = new Linear(0.1, 0.0, buf);
		pI2CInput -> connectTo(pTransform)-> connectTo(new SKOutputNumber(buf));
				sprintf(buf, "%sBearingBosch",config_path);
		pI2CInput = new I2CInput(0x60,26, buf, 16);
		pTransform = new Linear(0.0625, 0.0, buf);
		pI2CInput -> connectTo(pTransform)-> connectTo(new SKOutputNumber(buf));
			sprintf(buf, "%sBearing8",config_path);
		pI2CInput = new I2CInput(0x60,1, buf, 8);
		pTransform = new Linear(1.4117647, 0.0, buf);
		pI2CInput -> connectTo(pTransform)-> connectTo(new SKOutputNumber(buf));
		sprintf(buf, "%sTemp",config_path);
		pI2CInput = new I2CInput(0x60,24, buf, 16);
		pI2CInput -> connectTo(new SKOutputNumber(buf));
		sprintf(buf, "%sCalibration",config_path);
		pI2CInput = new I2CInput(0x60,30, buf, 8);
		pI2CInput -> connectTo(new SKOutputNumber(buf));
		sprintf(buf, "%sPitch",config_path);
		pI2CInput = new I2CInput(0x60,4, buf, 8);
		pI2CInput -> connectTo(new SKOutputNumber(buf));
		sprintf(buf, "%sXRoll",config_path);
		pI2CInput = new I2CInput(0x60,5, buf, 8);
		pI2CInput -> connectTo(new SKOutputNumber(buf));
	// Start the SensESP application running
	sensesp_app->enable();
});

/*
/vessels/<RegExp>/navigation/attitude
/vessels/<RegExp>/navigation/headingMagnetic
Units: rad (Radian)

Description: Current magnetic heading of the vessel

/vessels/<RegExp>/navigation/headingTrue
Units: rad (Radian)

Description: The current true heading of the vessel

/vessels/<RegExp>/navigation/attitude
Title: Attitude

Description: Vessel attitude: roll, pitch and yaw

Object value with properties

roll (rad)
pitch (rad)
yaw (rad)

/vessels/<RegExp>/navigation/rateOfTurn
Units: rad/s (Radian per second)

Description: Rate of turn (+ve is change to starboard). If the value is AIS RIGHT or LEFT, set to +-0.0206 rads and add warning in notifications


/vessels/<RegExp>/navigation/magneticVariation
Units: rad (Radian)

Description: The magnetic variation (declination) at the current position that must be added to the magnetic heading to derive the true heading. Easterly variations are positive and Westerly variations are negative (in Radians).

/vessels/<RegExp>/navigation/magneticVariationAgeOfService
Units: s (Second)

Description: Seconds since the 1st Jan 1970 that the variation calculation was made*/