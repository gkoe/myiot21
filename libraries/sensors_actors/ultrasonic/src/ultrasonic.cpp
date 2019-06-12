/**
 * @file ultrasonic.c
 *
 * ESP-IDF driver for ultrasonic range meters, e.g. HC-SR04, HY-SRF05 and so on
 *
 * Ported from esp-open-rtos
 * Copyright (C) 2016, 2018 Ruslan V. Uss <unclerus@gmail.com>
 * BSD Licensed as described in the file LICENSE
 */
#include "ultrasonic.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <sys/time.h>

#include <Logger.h>

#define TRIGGER_LOW_DELAY 4
#define TRIGGER_HIGH_DELAY 10
#define PING_TIMEOUT 6000
#define ROUNDTRIP 58

static portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

static inline uint32_t get_time_us()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_usec;
}

#define timeout_expired(start, len) ((uint32_t)(get_time_us() - (start)) >= (len))

#define RETURN_CRTCAL(MUX, RES)  \
    do                           \
    {                            \
        portEXIT_CRITICAL(&MUX); \
        return RES;              \
    } while (0)

void measureDistanceInLoopTask(void *pvParameter)
{
    Ultrasonic *ultrasonicPtr = (Ultrasonic *)pvParameter;
    uint32_t distance;
    esp_err_t errorCode;

    while (1)
    {
        errorCode = ultrasonicPtr->measureDistance(&distance);
        if (errorCode == ESP_OK)
        {
            ultrasonicPtr->setNextDistance(distance);
            // printf("Distance measured: %d\n", distance);
        }
        else
        {
            switch (errorCode)
            {
                case ESP_ERR_ULTRASONIC_PING:
                    printf("!!! Error reading ultrasonic, Cannot ping (device is in invalid state)\n");
                    Logger.error("ultrasonic;measureDistanceInLoopTask()", "Cannot ping (device is in invalid state)");
                    break;
                case ESP_ERR_ULTRASONIC_PING_TIMEOUT:
                    //printf("!!! Error reading ultrasonic, Ping timeout (no device found)\n");
                    break;
                case ESP_ERR_ULTRASONIC_ECHO_TIMEOUT:

                    printf("!!! Error reading ultrasonic, Echo timeout (i.e. distance too big)\n");
                    Logger.error("ultrasonic;measureDistanceInLoopTask()", "Echo timeout (i.e. distance too big)");
                    break;
                default:
                    printf("%d\n", errorCode);
            }
        }
        vTaskDelay(200 / portTICK_RATE_MS);
    }
}

Ultrasonic::Ultrasonic(gpio_num_t triggerPin, gpio_num_t echoPin, const char *thingName, const char *name, const char *unit, float threshold)
    : IotSensor(thingName, name, unit, threshold)
{
    gpio_set_direction(triggerPin, GPIO_MODE_OUTPUT);
    gpio_set_direction(echoPin, GPIO_MODE_INPUT);
    gpio_set_level(triggerPin, 0);
    _triggerPin = triggerPin;
    _echoPin = echoPin;
    for (int i = 0; i < 10; i++)
    {
        _lastDistances[i] = -1;
    }
    xTaskCreate(measureDistanceInLoopTask,   /* Task function. */
                "measureDistanceInLoopTask", /* String with name of task. */
                4096,                        /* Stack size in words. */
                this,                        /* Parameter passed as input of the task */
                1,                           /* Priority of the task. */
                NULL                         /* Task handle. */
    );
}

void Ultrasonic::setNextDistance(uint32_t distance)
{
    _lastDistances[_actIndex] = distance;
    _actIndex++;
    if (_actIndex >= 10)
    {
        _actIndex = 0;
    }
}

esp_err_t Ultrasonic::measureDistance(uint32_t *distance)
{

    portENTER_CRITICAL(&mux);

    // Ping: Low for 2..4 us, then high 10 us
    gpio_set_level(_triggerPin, 0);
    ets_delay_us(TRIGGER_LOW_DELAY);
    gpio_set_level(_triggerPin, 1);
    ets_delay_us(TRIGGER_HIGH_DELAY);
    gpio_set_level(_triggerPin, 0);

    // Previous ping isn't ended
    if (gpio_get_level(_echoPin))
        RETURN_CRTCAL(mux, ESP_ERR_ULTRASONIC_PING);

    // Wait for echo
    uint32_t start = get_time_us();
    while (!gpio_get_level(_echoPin))
    {
        if (timeout_expired(start, PING_TIMEOUT))
            RETURN_CRTCAL(mux, ESP_ERR_ULTRASONIC_PING_TIMEOUT);
    }

    // got echo, measuring
    uint32_t echo_start = get_time_us();
    uint32_t time = echo_start;
    uint32_t meas_timeout = echo_start + (MAX_DISTANCE * ROUNDTRIP);
    while (gpio_get_level(_echoPin))
    {
        time = get_time_us();
        if (timeout_expired(echo_start, meas_timeout))
            RETURN_CRTCAL(mux, ESP_ERR_ULTRASONIC_ECHO_TIMEOUT);
    }
    portEXIT_CRITICAL(&mux);

    *distance = (time - echo_start) / ROUNDTRIP;

    return ESP_OK;
}

float Ultrasonic::getAverageDistance()
{
    int minValue = 1000;
    int maxValue = 0;
    int validValues = 0;
    int sumOfValues = 0;
    for (int i = 0; i < 10; i++)
    {
        if (_lastDistances[i] != -1)
        {
            uint32_t value = _lastDistances[i];
            if (value > maxValue)
            {
                maxValue = value;
            }
            else if (value < minValue)
            {
                minValue = value;
            }
            sumOfValues += value;
            validValues++;
        }
    }
    if (validValues < 3)
        return -1;
    //printf("sumOfValues: %d, minValue: %d, maxValue: %d, validValues: %d\n", sumOfValues, minValue, maxValue, validValues);
    return (sumOfValues - minValue - maxValue) / ((float)validValues - 2);
}

/**
  measure() gets the measurment and set it.
*/
void Ultrasonic::measure()
{
    setMeasurement(getAverageDistance());
}
