#include <Arduino.h>
#include <IotSensor.h>


class SimplePir : public IotSensor
{
  public:
    SimplePir( int pin, int interval, const char *thingName, const char *name, 
                const char *unit, float threshold);
  	virtual void measure();

  private:
    unsigned long  _lastMotionDetectedMs;
    int _pin;
    int _intervalMs; 
    bool _isMotion; 
};