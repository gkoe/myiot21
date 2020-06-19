#pragma once
#include <list>

#include "driver/gpio.h"
#include <IotSensor.h>
#include "ds18x20.h"

static const int MAX_SENSORS = 8;

class Ds18b20Hub
{
public:
    const float DUMMY_TEMPERATURE = -9999.9;
    
    Ds18b20Hub(gpio_num_t ioPin);
    gpio_num_t _ioPin;
    float getTemperature(char *sensorAddress);

    ds18x20_addr_t _addresses[MAX_SENSORS];
    float _temperatures[MAX_SENSORS];
    int _sensorCount;

private:
};
