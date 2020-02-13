#pragma once

#include <Air_Sgp30.h>
#include <IotSensor.h>

class Sgp30_Tvoc : public IotSensor
{
public:
	Sgp30_Tvoc(Air_Sgp30* sgp30, const char* thingName, const char* name, const char* unit, float threshold, float minValue = 0.0, float maxValue = 100.0, bool getAverageValue = true);
	virtual void measure();
private:
	Air_Sgp30* _sgp30;
};
