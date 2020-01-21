#pragma once
#include <IotSensor.h>

class MultiPir : public IotSensor
{
public:
  MultiPir(gpio_num_t pins[], int pirsCount, int interval, const char *thingName, const char *name, const char *unit, float threshold,
           float minValue = -9999.9, float maxValue = 111111, bool getAverageValue = false);
  virtual void measure();
  int getPirStates();

  gpio_num_t *_pins;
  int _pirsCount;
  int _intervalMs;
  int _lastPirStates;
  int _actPirStates;

private:
};