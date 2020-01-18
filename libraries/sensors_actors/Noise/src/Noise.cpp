#include "Noise.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <sys/time.h>

#include <Logger.h>

const float CALIBRATION_DIVIDER = 2630.0;

long getMaxDeltaOfMeasurements(adc1_channel_t adcChannel)
{
    int minValue = 10000;
    int maxValue = -1;
    int value;
    for (int i = 0; i < 10000; i++)
    {
        value = adc1_get_raw(adcChannel);
        // printf("%d\n", value);
        if (value > maxValue)
        {
            maxValue = value;
        }
        if (value < minValue)
        {
            minValue = value;
        }
        // vTaskDelay(1 / portTICK_RATE_MS);
    }
    // printf("NoiseLog;min: %d; max: %d;%d\n", minValue, maxValue, maxValue-minValue);
    vTaskDelay(50 / portTICK_RATE_MS);
    return maxValue - minValue;
}

void measureNoiseInLoopTask(void *pvParameter)
{
    Noise *noisePtr = (Noise *)pvParameter;
    while (1)
    {
        long startTime = esp_timer_get_time() / 1000;
        int value = getMaxDeltaOfMeasurements(noisePtr->_adcChannel);
        long duration = esp_timer_get_time() / 1000 - startTime;
        noisePtr->_actNoise = value;
        // printf("!!! Value: %i; duration: %.ld\n", value, duration);
        // vTaskDelay(1000 / portTICK_RATE_MS);
    }
}

Noise::Noise(adc1_channel_t adcChannel, const char *thingName, const char *name, const char *unit, float threshold, float minValue, float maxValue, bool getAverageValue)
    : IotSensor(thingName, name, unit, threshold, minValue, maxValue, getAverageValue)
{
    _adcChannel = adcChannel;
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(adcChannel, ADC_ATTEN_DB_11);

    xTaskCreate(measureNoiseInLoopTask,   /* Task function. */
                "measurePowerInLoopTask", /* String with name of task. */
                4096,                     /* Stack size in words. */
                this,                     /* Parameter passed as input of the task */
                1,                        /* Priority of the task. */
                NULL                      /* Task handle. */
    );
}

/**
  measure() gets the measurement and set it.
*/
void Noise::measure()
{
    setMeasurement(_actNoise);
}
