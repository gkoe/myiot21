#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_system.h"

#include "MultiPir.h"

// bool getPirStates(gpio_num_t pin)
// {
//   return gpio_input_get() & (1 << pin);
// }

// long getMillis()
// {
//   return esp_timer_get_time() / 1000;
// }

void measurePirsInLoopTask(void *pvParameter)
{
  MultiPir *pirPtr = (MultiPir *)pvParameter;
  while (1)
  {
    pirPtr->_actPirStates = pirPtr->getPirStates();
    vTaskDelay(100 / portTICK_RATE_MS);
  }
}

MultiPir::MultiPir(gpio_num_t pins[], int pirsCount, int interval, const char *thingName, const char *name, const char *unit, float threshold,
                   float minValue, float maxValue, bool getAverageValue)
    : IotSensor(thingName, name, unit, threshold, minValue, maxValue, getAverageValue)
{
  _pirsCount = pirsCount;
  _pins = new gpio_num_t[pirsCount];
  for (int i = 0; i < pirsCount; i++)
  {
    _pins[i] = pins[i];
    gpio_pad_select_gpio(pins[i]);
    gpio_set_direction(pins[i], GPIO_MODE_INPUT);
    printf("PIR on IO %d initialized\n", pins[i]);
  }

  _lastPirStates = getPirStates();
  _intervalMs = interval * 1000;
  xTaskCreate(measurePirsInLoopTask,   /* Task function. */
              "measurePirsInLoopTask", /* String with name of task. */
              2048,                   /* Stack size in words. */
              this,                   /* Parameter passed as input of the task */
              1,                      /* Priority of the task. */
              NULL                    /* Task handle. */
  );
}

int MultiPir::getPirStates()
{
  int result = 0;
  int mask = 1;
  // printf("getPirStates; Length of Pir[]: %d\n", _pirsCount);
  for (size_t i = 0; i < _pirsCount; i++)
  {
    if (getPinState(_pins[i]))
    {
      result = result + mask;
      // printf("getPirStates; PIR on PIN %d is HIGH\n", _pins[i]);
      // return true;
    }
    mask = mask * 10;
  }
  // printf("getPirStates; PIR-States %d\n", result);
  return result;
  return 0;
}

void MultiPir::measure()
{
  setMeasurement(_actPirStates);
}
