#pragma once
#include "driver/gpio.h"
#include <IotSensor.h>
#include <driver/adc.h>

class Template : public IotSensor
{
public:
    Template(adc1_channel_t adcChannel, const char *thingName, const char *name, 
        const char *unit, float threshold, float minValue = -9999.9, float maxValue = 9999.9, bool getAverageValue = true);
    virtual void measure();

    adc1_channel_t _adcChannel;
    volatile float _actTemplate = -1;

private:
};
