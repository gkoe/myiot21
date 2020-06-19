#include <string.h>
#include "Ds18b20Hub.h"
#include "Ds18b20Sensor.h"
#include <Logger.h>

Ds18b20Sensor::Ds18b20Sensor(Ds18b20Hub *ds18b20Hub, const char *address, const char *thingName, const char *name, const char *unit,
							 float threshold, float minValue, float maxValue, bool getAverageValue)
	: IotSensor(thingName, name, unit, threshold, minValue, maxValue, getAverageValue)
{
	_ds18b20Hub = ds18b20Hub;
	strncpy(_address, address, 17);
	printf("Ds18b20Sensor, constructor, address: %s", _address);
}

void Ds18b20Sensor::measure()
{
	float temperature = _ds18b20Hub->getTemperature(_address);
	if (temperature > _ds18b20Hub->DUMMY_TEMPERATURE)
	{
		setMeasurement(temperature);
	}
	else
	{
		// printf("ERROR!! Ds18b20Sensor::measure(), no value for address: %f\n", _ds18b20Hub->DUMMY_TEMPERATURE);
	}
}
