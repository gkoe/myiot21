#include <stdio.h>
#include <stdbool.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <ultrasonic.h>

#include <EspConfig.h>
#include "Logger.h"
#include <LoggerTarget.h>
#include <SerialLoggerTarget.h>
#include <SystemService.h>
#include <Thing.h>


const char *SERIAL_LOGGER_TAG = "SLT";

#define TRIGGER_GPIO GPIO_NUM_18
#define ECHO_GPIO GPIO_NUM_19


extern "C"
{
  void app_main(void);
}

void app_main()
{
  char loggerMessage[LENGTH_LOGGER_MESSAGE];
  printf("===============\n");
  printf("Ultrasonic-Test\n");
  printf("===============\n");
  EspConfig.init();
  Logger.init("ThingTest");
  SerialLoggerTarget *serialLoggerTarget = new SerialLoggerTarget(SERIAL_LOGGER_TAG, LOG_LEVEL_VERBOSE);
  Logger.addLoggerTarget(serialLoggerTarget);
  SystemService.init();
  Logger.info("UltrasonicTest;app_main()", "Initialize Ultrasonic!");

  Ultrasonic *ultrasonic = new Ultrasonic(TRIGGER_GPIO, GPIO_NUM_19, "Ultrasonic", "waterlevel", "cm", 0.1);

  while (true)
  {
    float cm = ultrasonic->getAverageDistance();
    sprintf(loggerMessage, "Distance:  %.1f", cm);
    Logger.info("UltrasonicTest;app_main()", loggerMessage);
    vTaskDelay(5000 / portTICK_RATE_MS);
    SystemService.checkSystem();
  }
}



