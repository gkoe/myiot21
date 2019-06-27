#pragma once
#include "driver/gpio.h"
#include <IotSensor.h>


class HX711 : public IotSensor
{
private:
	// byte PD_SCK;	 // Power Down and Serial Clock Input Pin
	// byte DOUT;		 // Serial Data Output Pin
	// byte GAIN;		 // amplification factor
	// long OFFSET = 0; // used for tare weight
	// float SCALE = 1; // used to return weight in grams, kg, ounces, whatever

public:
	HX711(gpio_num_t dataOutPin, gpio_num_t clockPin, const char *thingName, const char *name, const char *unit, float threshold);

	gpio_num_t _dataPin;
	gpio_num_t _clockPin;
	volatile float _actWeight = -1;
	virtual void measure();

};

