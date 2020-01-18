#include <stdio.h>
#include <stdbool.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <Luminosity.h>

#include <EspConfig.h>
#include "Logger.h"
#include <LoggerTarget.h>
#include <SerialLoggerTarget.h>
#include <SystemService.h>
#include <Thing.h>


const char *SERIAL_LOGGER_TAG = "SLT";

#define I2C_SDA        GPIO_NUM_18
#define I2C_SCL        GPIO_NUM_19

extern "C"
{
  void app_main(void);
}

void app_main()
{
  char loggerMessage[LENGTH_LOGGER_MESSAGE];
  printf("=====================\n");
  printf("LuminositySensor-Test\n");
  printf("=====================\n");
  EspConfig.init();
  Logger.init("LuminositySensorTest");
  SerialLoggerTarget *serialLoggerTarget = new SerialLoggerTarget(SERIAL_LOGGER_TAG, LOG_LEVEL_VERBOSE);
  Logger.addLoggerTarget(serialLoggerTarget);
  SystemService.init();
  Logger.info("LuminositySensorTest;app_main()", "Initialize Luminositysensor!");

  Luminosity *luminosityPtr = new Luminosity(I2C_SDA, I2C_SCL, "Luminositysensor", "Luminosity", "Lux", 2, 0, 50000, false);

  while (true)
  {
    luminosityPtr->measure();
    float Luminosity = luminosityPtr->getLastMeasurement();
    sprintf(loggerMessage, "Luminosity:  %.1f", Luminosity);
    Logger.info("LuminositySensorTest;app_main()", loggerMessage);
    vTaskDelay(3000 / portTICK_RATE_MS);
    SystemService.checkSystem();
  }
}



