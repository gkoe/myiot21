#pragma once

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <Sensor.h>


class Mhz14a: public Sensor
{
  public:
  	Mhz14a(int rxPin, int txPin, const char* nodeName, const char* name, const char* unit, float threshold);
  	virtual void measure();
    float getCo2();
  private:
    SoftwareSerial* _softwareSerialPtr;
    unsigned long _lastMeasurementMilliSeconds=0;
    float _ppmCo2;
};

