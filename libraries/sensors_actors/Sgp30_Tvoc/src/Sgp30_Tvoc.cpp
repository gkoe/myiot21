#include "Sgp30_Tvoc.h"

Sgp30_Tvoc::Sgp30_Tvoc(Air_Sgp30* sgp30, const char* thingName, const char* name,const char* unit, float threshold, float minValue, float maxValue, bool getAverageValue) 
		:IotSensor( thingName, name, unit, threshold, minValue, maxValue, getAverageValue)
{
	_sgp30 = sgp30;
}

void Sgp30_Tvoc::measure()
{
	float tvoc = _sgp30->getTvoc();
	setMeasurement(tvoc);
}
