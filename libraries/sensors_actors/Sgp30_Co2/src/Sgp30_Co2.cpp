#include "Sgp30_Co2.h"

Sgp30_Co2::Sgp30_Co2(Air_Sgp30* sgp30, const char* thingName, const char* name,const char* unit, float threshold, float minValue, float maxValue, bool getAverageValue) 
		:IotSensor( thingName, name, unit, threshold, minValue, maxValue, getAverageValue)
{
	_sgp = sgp30;
}

void Sgp30_Co2::measure()
{
	float co2 = _sgp30->getCo2();
	setMeasurement(humidity);
}
