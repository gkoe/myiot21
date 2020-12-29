#include "Noise.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <sys/time.h>

#include <Logger.h>

/***
 * Es wird innerhalb des vorgegebenen Messfensters der minimale und der maximale 
 * Spannungswert des am AD-Wandler angeschlossenen Mikrofon gemessen.
 */
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
    // Um extreme Ausschläge zu vermeiden werden innerhalb des Zeitfensters
    // immer 1000 Proben genommen und aus diesen 1000 Proben der Min- und Maxwert ermittelt.
    // Nach 1000 Proben wird der Task für 10ms schlafen geschickt
    // Nach Ablauf des vorgegebenen Zeitfensters wird der Mittelwert der Minima und Maxima
    // ermittelt und zurückgemeldet
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
        if (round > 100)
        {
            sumMaxValue += maxValue;
            sumMinValue += minValue;
            maxValue = -1;
            minValue = 10000;
            vTaskDelay(10 / portTICK_RATE_MS); // Pegel alle 10ms messen
            round = 0;
            delaysCounter++;
        }
    }
    maxValue = sumMaxValue / delaysCounter;
    minValue = sumMinValue / delaysCounter;
    // printf("NoiseLog;min: %d; max: %d;%d;delaysCounter;%d\n", minValue, maxValue, maxValue - minValue, delaysCounter);
    vTaskDelay(1000 / portTICK_RATE_MS);
    return maxValue - minValue;
}

/**
 * Die Messwerte werden im eigenen Task entsprechend dem vorgegebenen Zeitfenster ermittelt
 * und im privaten Feld _actNoise deponiert.
 */
void measureNoiseInLoopTask(void *pvParameter)
{
    Noise *noisePtr = (Noise *)pvParameter;
    while (1)
    {
        long startTime = esp_timer_get_time() / 1000;
        int value = getMaxDeltaOfMeasurements(noisePtr->_adcChannel, noisePtr->_measurementWindowMs);
        // long duration = esp_timer_get_time() / 1000 - startTime;
        noisePtr->setMeasurement(value);
        // noisePtr->_actNoise = value;
        // printf("!!! Noise; Value: %i; duration: %.ld\n", value, duration);
        // vTaskDelay(1000 / portTICK_RATE_MS);
    }
}

Noise::Noise(adc1_channel_t adcChannel, int measurementWindowMs, const char *thingName, const char *name,
             const char *unit, float threshold, float minValue, float maxValue, bool getAverageValue)
    : IotSensor(thingName, name, unit, threshold, minValue, maxValue, getAverageValue)
{
    _adcChannel = adcChannel;
    _measurementWindowMs = measurementWindowMs;             // Zeitfenster für Messung: zu klein ==> extrem viele Messwerte
                                                            // zu groß ==> lange Reaktionszeit
    adc1_config_width(ADC_WIDTH_BIT_12);                    // maximal 4096 verschiedene Werte
    adc1_config_channel_atten(adcChannel, ADC_ATTEN_DB_11); // Dämpfungsfaktor, damit Mikro nicht übersteuert

    xTaskCreate(measureNoiseInLoopTask,   /* Task function. */
                "measureNoiseInLoopTask", /* String with name of task. */
                4096,                     /* Stack size in words. */
                this,                     /* Parameter passed as input of the task */
                1,                        /* Priority of the task. */
                NULL                      /* Task handle. */
    );
}

/**
  measure() wird von Mainloop zyklisch aufgerufen und 
  setzt den zu übertragenen Messwert des IotSensors
*/
void Noise::measure()
{
    //setMeasurement(_actNoise);
}
