#pragma once
#include "driver/gpio.h"
#include <IotSensor.h>
#include <driver/adc.h>

class Noise : public IotSensor
{
public:
    Noise(adc1_channel_t adcChannel, const char *thingName, const char *name, const char *unit, float threshold, float minValue = -9999.9, float maxValue = 9999.9, bool getAverageValue = true);


    adc1_channel_t _adcChannel;
    volatile float _actNoise = -1;
    virtual void measure();

private:

};
