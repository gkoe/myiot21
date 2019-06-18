#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include <driver/adc.h>

extern "C"
{
  void app_main(void);
}

long getMillis()
{
  return esp_timer_get_time() / 1000;
}

bool getPinState(gpio_num_t pin)
{
  return gpio_input_get() & (1 << pin);
}

int findMaxInOnePeriode()
{
  long start = esp_timer_get_time();
  int value;
  int maxValue = 0;
  int samples = 0;
  while (esp_timer_get_time() - start < 200000) // 20 ms lang abtasten
  {
    value = adc1_get_raw(ADC1_CHANNEL_6);
    samples++;
    // printf("          Value: %i\n", value);
    if (value > maxValue)
    {
      maxValue = value;
    }
    // vTaskDelay(1);
  }
  printf("Samples: %i, ", samples);
  return maxValue;
}

long getSumOverDuration()
{
  long start = esp_timer_get_time();
  int value;
  int samples = 0;
  long sum = 0;
  for (int i = 0; i < 5000; i++)
  {
    value = adc1_get_raw(ADC1_CHANNEL_6);
    samples++;
    sum+=value;
  }
  
  // while (esp_timer_get_time() - start < 200000) // 200 ms lang abtasten
  // {
  //   value = adc1_get_raw(ADC1_CHANNEL_6);
  //   samples++;
  //   sum+=value;
  //   // vTaskDelay(1);
  // }
  printf("Samples: %i, ", samples);
  return sum;
}

void app_main()
{
  // int lastChangeMs;
  // bool lastState;

  printf("========\n");
  printf("ADC-Test\n");
  printf("========\n");

  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_0);
  while (true)
  {
    long startTime = getMillis();
    long value = getSumOverDuration();
    long duration = getMillis() -startTime;
    float power = value * 60 / 790000;
    printf("MaxValue: %ld, Duration: %ld, power: %.1f\n", value, duration, power);
    vTaskDelay(1000 / portTICK_RATE_MS);
  }
}
