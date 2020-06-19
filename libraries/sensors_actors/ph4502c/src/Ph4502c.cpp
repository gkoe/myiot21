#include "Ph4502c.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <sys/time.h>

#include <Logger.h>

const adc1_channel_t adcChannel = ADC1_CHANNEL_6; // GPIO34

const float V9_18 = 1660.0; // mV
const float V6_86 = 1920.0;
const float V4_01 = 2220.0;
const float V0_00 = 2654; // 2,654V bei ph-Wert 0
const float V24_50 = 0.0; // 0V bei ph-Wert 24,5

// Geradengleichung ph-Wert = k * mV + d
//    mV: bei Spannungsteiler 1:1 (22 kOhm) und 11 dB Dämpfung (ADC_ATTEN_DB_11)
//    d ermittelt aus den Versuchen ==> 24.5

const float k = (9.18 - 4.01) / ((V9_18 - V4_01) / 1000.0);
const float d = 9.18 - (k*V9_18/1000.0);

const float k = -9.22 / 1000; // mv/ph-Wert
const float d = 24.5;

// /**
//  * Liefert cirka 1000 Messwerte innerhalb von 40 ms (Zwei Perioden).
//  * Führt zu ausreichender Genauigkeit
//  */
// long getSumOf1000PhMeasurementsIn40ms(adc1_channel_t adcChannel, int *maxValue)
// {
//     *maxValue = -1;
//     int value;
//     int samples = 0;
//     long sum = 0;
//     for (int i = 0; i < 1000; i++) // 5000
//     {
//         value = adc1_get_raw(adcChannel);
//         if (value > *maxValue)
//         {
//             *maxValue = value;
//         }
//         samples++;
//         sum += value;
//     }
//     return sum;
// }

/**
 * Ermittelt alle Sekunden den aktuellen PH-Wert
 */
void measurePhInLoopTask(void *pvParameter)
{
    Ph4502c *phPtr = (Ph4502c *)pvParameter;
    while (1)
    {
        // long startTime = esp_timer_get_time() / 1000;
        //long value = getSumOf1000PhMeasurementsIn40ms(phPtr->_adcChannel, &maxValue) / 1000.0;
        int value = adc1_get_raw(adcChannel);
        // long duration = esp_timer_get_time() / 1000 - startTime;
        float ph = k * value + d;
        printf("Value: %i, ph: %.1f\n", value, ph);
        phPtr->_actPh = ph;
        vTaskDelay(10000 / portTICK_RATE_MS);
    }
}

Ph4502c::Ph4502c(adc1_channel_t adcChannel, const char *thingName, const char* name,const char* unit, float threshold, float minValue, float maxValue, bool getAverageValue) 
    : IotSensor(thingName, name, unit, threshold, minValue, maxValue, getAverageValue)
{
    _adcChannel = adcChannel;
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(adcChannel, ADC_ATTEN_DB_11);

    xTaskCreate(measurePhInLoopTask,   /* Task function. */
                "measurePhInLoopTask", /* String with name of task. */
                4096,                  /* Stack size in words. */
                this,                  /* Parameter passed as input of the task */
                1,                     /* Priority of the task. */
                NULL                   /* Task handle. */
    );
}

/**
  measure() gets the measurment and set it.
*/
void Ph4502c::measure()
{
    setMeasurement(_actPh);
}
