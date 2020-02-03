#pragma once
#include "driver/gpio.h"
#include <IotSensor.h>
#include <driver/adc.h>

class Noise : public IotSensor
{
public:
    Noise(adc1_channel_t adcChannel, int measurementWindowMs, const char *thingName, const char *name, 
        const char *unit, float threshold, float minValue = -9999.9, float maxValue = 9999.9, bool getAverageValue = true);
    virtual void measure();

private:
    adc1_channel_t _adcChannel;
    int _measurementWindowMs;
    volatile float _actNoise = -1;
};
