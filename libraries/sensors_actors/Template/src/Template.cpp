#include "Template.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <sys/time.h>

#include <Logger.h>

/**
 * Die Messwerte werden im eigenen Task entsprechend dem vorgegebenen Zeitfenster ermittelt
 * und im privaten Feld _actTemplate deponiert.
 */
void measureTemplateInLoopTask(void *pvParameter)
{
    Template *templatePtr = (Template *)pvParameter;
    while (1)
    {
        // templatePtr->_actTemplate = getMeasuredValue();
        vTaskDelay(5000 / portTICK_RATE_MS);
    }
}

Template::Template(adc1_channel_t adcChannel, const char *thingName, const char *name, const char *unit, float threshold, float minValue, float maxValue, bool getAverageValue)
    : IotSensor(thingName, name, unit, threshold, minValue, maxValue, getAverageValue)
{
    _adcChannel = adcChannel;
    adc1_config_width(ADC_WIDTH_BIT_12);  // maximal 4096 verschiedene Werte
    adc1_config_channel_atten(adcChannel, ADC_ATTEN_DB_11);  // Dämpfungsfaktor, damit Mikro nicht übersteuert

    xTaskCreate(measureTemplateInLoopTask,   /* Task function. */
                "measurePowerInLoopTask", /* String with name of task. */
                4096,                     /* Stack size in words. */
                this,                     /* Parameter passed as input of the task */
                1,                        /* Priority of the task. */
                NULL                      /* Task handle. */
    );
}

/**
  measure() wird zyklisch aufgerufen und setzt den zu übertragenen Messwert des IotSensors
*/
void Template::measure()
{
    setMeasurement(_actTemplate);
}
