#include "Mhz14a.h"

Mhz14a::Mhz14a(int rxPin, int txPin, const char *nodeName, const char *name, const char *unit, float threshold) : Sensor(nodeName, name, unit, threshold)
{
  _softwareSerialPtr = new SoftwareSerial(rxPin, txPin, false, 64U);
}

const uint8_t cmd_measure_co2[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
float Mhz14a::getCo2()
{
  if (millis() - _lastMeasurementMilliSeconds > 1000)
  { // nur alle Sekunden messen
    _lastMeasurementMilliSeconds = millis();
    uint8_t response[9];
    _softwareSerialPtr->write(cmd_measure_co2, 9);
    if (_softwareSerialPtr->readBytes(response, 9) == 9)
    {
      int responseHigh = (int)response[2];
      int responseLow = (int)response[3];
      _ppmCo2 = (256 * responseHigh) + responseLow;
    }
    else
    {
      _ppmCo2 = -1;
    }
  }
  return _ppmCo2;
}

void Mhz14a::measure()
{
  float co2 = getCo2();
  setMeasurement(co2);
}
