#include "Bmp280_Pressure.h"
#include <Logger.h>

Bmp280_Pressure::Bmp280_Pressure(Bmp280Sensor* bmp, const char* thingName, const char* name,const char* unit, float threshold) 
		:IotSensor( thingName, name, unit, threshold)
{
	_bmp = bmp;
}

void Bmp280_Pressure::measure()
{
	float pressure = _bmp->getPressure();
	if(pressure < 0.0 || pressure > 1500.0){
		return;
	}
	setMeasurement(pressure);
}
