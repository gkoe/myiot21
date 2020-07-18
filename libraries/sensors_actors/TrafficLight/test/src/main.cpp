#include <stdio.h>
#include <stdbool.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <EspConfig.h>
#include "Logger.h"
#include <LoggerTarget.h>
#include <SerialLoggerTarget.h>
#include <SystemService.h>
#include <Thing.h>
#include <TrafficLight.h>

static const gpio_num_t NEOPIXEL_GPIO = GPIO_NUM_18;
const char *SERIAL_LOGGER_TAG = "SLT";

extern "C"
{
  void app_main(void);
}

void app_main()
{
  char loggerMessage[LENGTH_LOGGER_MESSAGE];
  printf("=================\n");
  printf("TrafficLight-Test\n");
  printf("=================\n");
  EspConfig.init();
  Logger.init("TrafficLightTest");
  SerialLoggerTarget *serialLoggerTarget = new SerialLoggerTarget(SERIAL_LOGGER_TAG, LOG_LEVEL_VERBOSE);
  Logger.addLoggerTarget(serialLoggerTarget);
  SystemService.init();
  Logger.info("TrafficLightTest;app_main()", "Initialize TrafficLight!");

  TrafficLight *trafficLight = new TrafficLight(NEOPIXEL_GPIO, "TestThing", "neopixel");
  Logger.info("TrafficLightTest;app_main()", "Before Loop");

  int round = 0;
  while (true)
  {
    if (round % 5000 == 0)
    {
      round = 0;
      sprintf(loggerMessage, "TrafficLight set");
      Logger.info("TrafficLightTest;app_main()", loggerMessage);
      trafficLight->setActor("GREEN");
    }
    else if(round % 4000 == 0){
      sprintf(loggerMessage, "TrafficLight set");
      Logger.info("TrafficLightTest;app_main()", loggerMessage);
      trafficLight->setActor("YELLOW");
    }
    else if(round % 3000 == 0){
      sprintf(loggerMessage, "TrafficLight set");
      Logger.info("TrafficLightTest;app_main()", loggerMessage);
      trafficLight->setActor("ORANGE");
    }
    else if(round % 2000 == 0){
      sprintf(loggerMessage, "TrafficLight set");
      Logger.info("TrafficLightTest;app_main()", loggerMessage);
      trafficLight->setActor("RED");
    }
    else if(round % 1000 == 0){
      sprintf(loggerMessage, "TrafficLight set");
      Logger.info("TrafficLightTest;app_main()", loggerMessage);
      trafficLight->setActor("OFF");
    }
    vTaskDelay(20 / portTICK_RATE_MS);
    SystemService.checkSystem();
    round++;
  }
}
