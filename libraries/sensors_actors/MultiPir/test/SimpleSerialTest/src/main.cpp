#include <stdio.h>
#include <stdbool.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/adc.h>

#include <MultiPir.h>

#include <EspConfig.h>
#include "Logger.h"
#include <LoggerTarget.h>
#include <SerialLoggerTarget.h>
#include <SystemService.h>
#include <Thing.h>


const char *SERIAL_LOGGER_TAG = "SLT";

gpio_num_t PIR_PINS[] = {GPIO_NUM_25,GPIO_NUM_26,GPIO_NUM_27,GPIO_NUM_14 };

extern "C"
{
  void app_main(void);
}

void app_main()
{
  char loggerMessage[LENGTH_LOGGER_MESSAGE];
  printf("===================\n");
  printf("MultiPirSensor-Test\n");
  printf("===================\n");
  EspConfig.init();
  Logger.init("MultiPirTest");
  SerialLoggerTarget *serialLoggerTarget = new SerialLoggerTarget(SERIAL_LOGGER_TAG, LOG_LEVEL_VERBOSE);
  Logger.addLoggerTarget(serialLoggerTarget);
  SystemService.init();
  Logger.info("MultiPirSensorTest;app_main()", "Initialize MultiPirsensor!");

  MultiPir *multiPirPtr = new MultiPir(PIR_PINS, 4, 60, "MultiPirsensor", "MultiPir", "",0.5,0,1111,false);

  while (true)
  {
    multiPirPtr->measure();
    float MultiPir = multiPirPtr->getLastMeasurement();
    sprintf(loggerMessage, "MultiPir:  %.1f", MultiPir);
    Logger.info("MultiPirSensorTest;app_main()", loggerMessage);
    vTaskDelay(1000 / portTICK_RATE_MS);
    SystemService.checkSystem();
  }
}



