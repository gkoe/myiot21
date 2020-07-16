#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_system.h"

#include "SimplePir.h"

bool getPinState(gpio_num_t pin)
{
  return gpio_input_get() & (1 << pin);
}

long getMillis()
{
  return esp_timer_get_time() / 1000;
}

SimplePir::SimplePir(gpio_num_t pin, int interval, const char *thingName,
                     const char *name, const char *unit) : IotSensor(thingName, name, unit, 0.1, 0.0, 1.0, false)
{
  _pin = pin;
  gpio_pad_select_gpio(_pin);
  gpio_set_direction(_pin, GPIO_MODE_INPUT);
  gpio_set_pull_mode(_pin, GPIO_PULLUP_PULLDOWN); // PULLUP funktioniert nicht
  _isMotion = getPinState(_pin);
  _lastMotionDetectedMs = getMillis();
  _intervalMs = interval * 1000;
}

void SimplePir::measure()
{
  bool isNowMotion = getPinState(_pin);
  if (!isNowMotion && !_isMotion)
  { // keine Änderung
    return;
  }
  if (isNowMotion == true)
  {                                      // neue Bewegung erkannt
    _lastMotionDetectedMs = getMillis(); // Delay gilt ab jetzt
    // printf("SimplePir;IsMotion, millis: %ld isMotion: %d\n", _lastMotionDetectedMs, _isMotion);
    if (_isMotion == false)
    {
      printf("SimplePir;New motion\n");
      _isMotion = true;
      setMeasurement(1.0);
    }
  }
  else
  { // isNowMotion == false && _pirState == true  ==> Zeitablauf
    long timeout = _lastMotionDetectedMs + _intervalMs;
    if (getMillis() > timeout)
    {
      printf("SimplePir;Motion ended; millis: %ld, timeout: %ld\n", getMillis(), timeout);
      _isMotion = false;
      setMeasurement(0.0);
    }
  }
}
