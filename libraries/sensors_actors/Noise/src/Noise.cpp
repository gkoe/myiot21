#include "Noise.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <sys/time.h>

#include <Logger.h>

const float CALIBRATION_DIVIDER = 2630.0;

long getMaxDeltaOfMeasurements(adc1_channel_t adcChannel, int measurementWindowMs)
{
    int sumMinValue = 0;
    int sumMaxValue = 0;
    int minValue = 10000;
    int maxValue = -1;
    int value;
    int delaysCounter = 0;
    long startTimeMs = esp_timer_get_time() / 1000;
    int round = 0;
    while (esp_timer_get_time() / 1000 - startTimeMs < measurementWindowMs)
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
        round++;
        if (round > 1000)
        {
            sumMaxValue+=maxValue;
            sumMinValue+=minValue;
            maxValue=-1;
            minValue=10000;
            vTaskDelay(10 / portTICK_RATE_MS);
            round = 0;
            delaysCounter++;
        }
    }
    maxValue = sumMaxValue/delaysCounter;
    minValue = sumMinValue/delaysCounter;
    // printf("NoiseLog;min: %d; max: %d;%d;delaysCounter;%d\n", minValue, maxValue, maxValue-minValue,delaysCounter);
    vTaskDelay(50 / portTICK_RATE_MS);
    return maxValue - minValue;
}

void measureNoiseInLoopTask(void *pvParameter)
{
    Noise *noisePtr = (Noise *)pvParameter;
    while (1)
    {
        // long startTime = esp_timer_get_time() / 1000;
        int value = getMaxDeltaOfMeasurements(noisePtr->_adcChannel, noisePtr->_measurementWindowMs);
        // long duration = esp_timer_get_time() / 1000 - startTime;
        noisePtr->_actNoise = value;
        // printf("!!! Value: %i; duration: %.ld\n", value, duration);
        // vTaskDelay(1000 / portTICK_RATE_MS);
    }
}

Noise::Noise(adc1_channel_t adcChannel, int measurementWindowMs, const char *thingName, const char *name, const char *unit, float threshold, float minValue, float maxValue, bool getAverageValue)
    : IotSensor(thingName, name, unit, threshold, minValue, maxValue, getAverageValue)
{
    _adcChannel = adcChannel;
    _measurementWindowMs = measurementWindowMs;
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
