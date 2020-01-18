#pragma once
#include "driver/gpio.h"
#include <IotSensor.h>

class Luminosity : public IotSensor
{
public:
    Luminosity(gpio_num_t sda, gpio_num_t scl, const char *thingName, const char *name, const char *unit, float threshold, 
                float minValue = -9999.9, float maxValue = 9999.9, bool getAverageValue = true);


    gpio_num_t _sda;
    gpio_num_t _scl;
    volatile float _actLux = -1;
    virtual void measure();

private:

};
