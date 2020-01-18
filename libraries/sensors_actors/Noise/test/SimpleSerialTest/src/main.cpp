#include <stdio.h>
#include <stdbool.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/adc.h>

#include <Noise.h>

#include <EspConfig.h>
#include "Logger.h"
#include <LoggerTarget.h>
#include <SerialLoggerTarget.h>
#include <SystemService.h>
#include <Thing.h>


const char *SERIAL_LOGGER_TAG = "SLT";

const adc1_channel_t adcChannel = ADC1_CHANNEL_5;  // GPIO33

extern "C"
{
  void app_main(void);
}

void app_main()
{
  char loggerMessage[LENGTH_LOGGER_MESSAGE];
  printf("================\n");
  printf("NoiseSensor-Test\n");
  printf("================\n");
  EspConfig.init();
  Logger.init("NoiseSensorTest");
  SerialLoggerTarget *serialLoggerTarget = new SerialLoggerTarget(SERIAL_LOGGER_TAG, LOG_LEVEL_ERROR);
  Logger.addLoggerTarget(serialLoggerTarget);
  SystemService.init();
  Logger.info("NoiseSensorTest;app_main()", "Initialize Noisesensor!");

  Noise *noisePtr = new Noise(adcChannel, "noisesensor", "noise", "db", 20, 0, 5000, false);

  while (true)
  {
    noisePtr->measure();
    float noise = noisePtr->getLastMeasurement();
    sprintf(loggerMessage, "Noise:  %.1f", noise);
    Logger.info("NoiseSensorTest;app_main()", loggerMessage);
    vTaskDelay(1000 / portTICK_RATE_MS);
    SystemService.checkSystem();
  }
}



