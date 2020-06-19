#pragma once

#include <Ds18b20Hub.h>
#include <IotSensor.h>

class Ds18b20Sensor : public IotSensor
{
public:
	Ds18b20Sensor(Ds18b20Hub* ds18b20Hub, const char* address, const char* thingName, const char* name, const char* unit,
	 float threshold, float minValue = -99.9, float maxValue = 999.9, bool getAverageValue = true);

	virtual void measure();
	void setTemperature(float temperature);

private:
	Ds18b20Hub* _ds18b20Hub;
	char _address[20];
};
